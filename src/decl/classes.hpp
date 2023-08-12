#pragma once

#include "./mutex_attribute_recursive.hpp"
#include "./class.hpp"

#include "decl/blocky_memory.hpp"
#include "decl/lib/java/lang/class_loader.hpp"
#include "decl/lib/java/lang/linkage_error.hpp"
#include "decl/primitives.hpp"
#include "decl/object.hpp"

#include <list.hpp>
#include <optional.hpp>
#include <ranges.hpp>
#include <on_scope_exit.hpp>

#include <posix/memory.hpp>
#include <class_file/reader.hpp>

struct class_and_initiating_loaders {
	c class_;
	// TODO make resizeable
	list<blocky_memory<reference, 16>> initiating_loaders;

	template<typename... Args>
	requires constructible_from<c, Args...>
	class_and_initiating_loaders(
		Args&&... args
	) :
		class_ { forward<Args>(args)... },
		initiating_loaders{}
	{
		initiating_loaders.emplace_back<reference>(class_.defining_loader());
	}

	void record_as_initiating(j::c_loader* cl) {
		if(loader_is_recorded_as_initiating(cl)) {
			print::err("class-loader is already recorded as initiating\n");
			posix::abort();
		}
		initiating_loaders.emplace_back<reference>(*cl);
	}

	bool loader_is_recorded_as_initiating(j::c_loader* cl) const {
		for(const reference& l : initiating_loaders) {
			if(l.object_ptr() == cl) {
				return true;
			}
		}
		return false;
	}

	void reset() {
		initiating_loaders.clear();
		class_.reset();
	}
};

static struct classes :
	private list<blocky_memory<class_and_initiating_loaders, 256>>
{

private:
	using base_type = list<blocky_memory<class_and_initiating_loaders, 256>>;
	using base_type::base_type;

	body<posix::mutex> mutex_ = posix::create_mutex(mutex_attribute_recursive);
public:

	~classes() {
		for(class_and_initiating_loaders& c_and_cl : *this) {
			c_and_cl.reset();
		}
	}

private:
	void mark_class_loader_as_initiating_for_class(
		c& c, j::c_loader* cl
	) {
		for(class_and_initiating_loaders& c_and_cl : *this) {
			if(&c_and_cl.class_ == &c) {
				c_and_cl.record_as_initiating(cl);
				return;
			}
		}
		posix::abort();
	}
public:

	template<basic_range Name>
	optional<c&> try_find_class_which_loading_was_initiated_by(
		Name&& name, j::c_loader* cl
	) {
		mutex_->lock();
		on_scope_exit unlock_classes_mutex { [&] {
			mutex_->unlock();
		}};

		optional<class_and_initiating_loaders&> possible_c_and_il
			= this->try_find_first_satisfying(
				[&](class_and_initiating_loaders& c_and_l) {
					bool same_name =
						c_and_l.class_.name().has_equal_size_and_elements(name);
					
					bool cl_is_initiating =
						c_and_l.loader_is_recorded_as_initiating(cl);

					return same_name && cl_is_initiating;
				}
			);

		if(possible_c_and_il.has_no_value()) {
			return {};
		}

		return possible_c_and_il.get().class_;
	}

	template<typename... Args>
	c& emplace_back(Args&&... args) {
		class_and_initiating_loaders& c_and_il = base_type::emplace_back(
			forward<Args>(args)...
		);
		return c_and_il.class_;
	}

	template<basic_range Name>
	expected<c&, reference>
	try_load_non_array_class_by_bootstrap_class_loader(
		Name&& name
	);

	template<basic_range Name>
	c& load_non_array_class_by_bootstrap_class_loader(Name&& name) {
		expected<c&, reference> result
			= try_load_non_array_class_by_bootstrap_class_loader(
				forward<Name>(name)
			);
		if(result.is_unexpected()) {
			print::err(
				"couldn't load non-array class ", name,
				" by bootstrap class-loader\n"
			);
			posix::abort();
		}
		return result.get_expected();
	}

	template<basic_range Name>
	expected<c&, reference>
	try_load_non_array_class(Name&& name, j::c_loader* l) {
		return l == nullptr ?
			try_load_non_array_class_by_bootstrap_class_loader(
				forward<Name>(name)
			)
			:
			try_load_non_array_class_by_user_class_loader(
				forward<Name>(name), move(l)
			);
	}

	template<basic_range Name>
	c& load_non_array_class(
		Name&& name, j::c_loader* l
	) {
		expected<c&, reference> r = try_load_non_array_class(
			forward<Name>(name), l
		);
		if(r.is_unexpected()) {
			print::err("couldn't load non-array class ", name, "\n");
			posix::abort();
		}
		return r.get_expected();
	}

	template<basic_range Name>
	expected<c&, reference>
	try_load_non_array_class_by_user_class_loader(
		Name&& name, j::c_loader* l
	);

	template<basic_range Name>
	expected<c&, reference> try_load_array_class(
		Name&& name, j::c_loader* l
	);

	template<basic_range Name>
	c& load_array_class(
		Name&& name, j::c_loader* l
	) {
		expected<c&, reference> r = try_load_array_class(
			forward<Name>(name), l
		);
		if(r.is_unexpected()) {
			print::err("couldn't load array class ", name, "\n");
			posix::abort();
		}
		return r.get_expected();
	}

	template<basic_range Name>
	expected<c&, reference> try_define_class(
		Name&& name,
		posix::memory<> bytes,
		j::c_loader* defining_loader // L
	);

	template<basic_range Name>
	expected<c&, reference> try_define_array_class(
		Name&& name,
		j::c_loader* defining_loader
	);

	inline c& define_primitive_class(
		class_file::constant::utf8 name,
		const utf8::unit& ch
	);

	template<basic_range Name>
	expected<c&, reference>
	try_load_class_by_bootstrap_class_loader(Name&& name) {
		return range{name}.starts_with('[') ?
			try_load_array_class(forward<Name>(name), nullptr)
			:
			try_load_non_array_class_by_bootstrap_class_loader(
				forward<Name>(name)
			);
	}

	template<basic_range Name>
	c& load_class_by_bootstrap_class_loader(Name&& name) {
		expected<c&, reference> result
			= try_load_class_by_bootstrap_class_loader(forward<Name>(name));
		if(result.is_unexpected()) {
			print::err(
				"couldn't load class ", name, " by bootstrap class-loader\n"
			);
			posix::abort();
		}
		return result.get_expected();
	}

	template<basic_range Name>
	expected<c&, reference>
	try_load_class(Name&& name, j::c_loader* class_loader) {
		if(range{name}.starts_with('[')) {
			return try_load_array_class(
				forward<Name>(name), class_loader
			);
		}

		return class_loader == nullptr ?
			try_load_non_array_class_by_bootstrap_class_loader(
				forward<Name>(name)
			)
			:
			try_load_non_array_class_by_user_class_loader(
				forward<Name>(name), class_loader
			);
	}

	template<typename Name, typename Handler>
	expected<c&, reference> lock_or_throw_linkage_error(
		Name&& class_name,
		j::c_loader* defining_loader,
		Handler&& handler
	) {
		mutex_->lock();
		on_scope_exit unlock_mutex { [&] { mutex_->unlock(); }};

		{
			optional<c&> c = try_find_class_which_loading_was_initiated_by(
				class_name,
				defining_loader
			);
			if(c.has_value()) {
				return unexpected { try_create_linkage_error().get() };
			}
		}

		return handler();
	}

} classes {};

#include "./classes.inc/define_class.hpp"
#include "./classes.inc/define_array_class.hpp"
#include "./classes.inc/define_primitive_class.hpp"
#include "./classes.inc/load_array_class.hpp"
#include "./classes.inc/load_non_array_class_by_bootstrap_class_loader.hpp"
#include "./classes.inc/load_non_array_class_by_user_class_loader.hpp"
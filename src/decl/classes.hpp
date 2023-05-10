#pragma once

#include "./mutex_attribute_recursive.hpp"
#include "./class.hpp"

#include "execution/info.hpp"
#include "executable_path.hpp"
#include "lib/java/lang/string.hpp"
#include "lib/java/lang/class_loader.hpp"
#include "lib/java/lang/class.hpp"
#include "lib/java/lang/object.hpp"
#include "lib/java/lang/class_not_found_exception.hpp"
#include "lib/java/lang/linkage_error.hpp"
#include "execute.hpp"
#include "primitives.hpp"
#include "try_load_class_file_data_at.hpp"
#include "class/bootstrap_methods.hpp"
#include "object.hpp"

#include <list.hpp>
#include <optional.hpp>
#include <ranges.hpp>

#include <posix/memory.hpp>
#include <class_file/reader.hpp>

struct class_and_initiating_loaders {
	c class_;
	// TODO make resizeable
	list<posix::memory_for_range_of<reference>> initiating_loaders;

	template<typename... Args>
	class_and_initiating_loaders(
		Args&&... args
	) :
		class_ { forward<Args>(args)... },
		initiating_loaders {
			posix::allocate_memory_for<reference>(16)
		}
	{
		initiating_loaders.emplace_back(class_.defining_loader());
	}

	void record_as_initiating(object* cl) {
		if(loader_is_recorded_as_initiating(cl)) {
			print::err("class-loader is already recorded as initiating\n");
			posix::abort();
		}
		initiating_loaders.emplace_back(*cl);
	}

	bool loader_is_recorded_as_initiating(object* cl) const {
		for(reference& l : initiating_loaders) {
			if(l.object_ptr() == cl) {
				return true;
			}
		}
		return false;
	}
};

static struct classes :
	private list<posix::memory_for_range_of<class_and_initiating_loaders>>
{
private:
	using base_type = list<
		posix::memory_for_range_of<class_and_initiating_loaders>
	>;
	using base_type::base_type;

	body<posix::mutex> mutex_ = posix::create_mutex(mutex_attribute_recursive);
public:

	~classes() {
		for(class_and_initiating_loaders& c : *this) {
			c.class_.destruct_declared_static_fields_values();
		}
	}

private:
	void mark_class_loader_as_initiating_for_class(c& c, object* cl) {
		for(auto& c_and_cl : *this) {
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
		Name&& name, object* class_loader
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
						c_and_l.loader_is_recorded_as_initiating(class_loader);

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
	try_load_non_array_class_by_user_class_loader(
		Name&& name, object* l_ref
	);

	template<basic_range Name>
	expected<c&, reference> try_load_array_class(
		Name&& name, object* l_ref
	);

	template<basic_range Name>
	c& load_array_class(
		Name&& name, object* l_ref
	) {
		expected<c&, reference> r = try_load_array_class(
			forward<Name>(name), move(l_ref)
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
		posix::memory_for_range_of<uint8> bytes,
		object* defining_loader // L
	);

	template<basic_range Name>
	c& define_array_class(Name&& name, object* defining_loader);

	template<basic_range Name>
	c& define_primitive_class(Name&& name, char ch);

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
	try_load_non_array_class(Name&& name, object* l_ref) {
		return l_ref == nullptr ?
			try_load_non_array_class_by_bootstrap_class_loader(
				forward<Name>(name)
			)
			:
			try_load_non_array_class_by_user_class_loader(
				forward<Name>(name), move(l_ref)
			);
	}

	template<basic_range Name>
	expected<c&, reference>
	try_load_class(Name&& name, object* class_loader) {
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

} classes{ posix::allocate_memory_for<class_and_initiating_loaders>(65536) };

#include "./classes.inc/define_array_class.hpp"
#include "./classes.inc/define_class.hpp"
#include "./classes.inc/define_primitive_class.hpp"
#include "./classes.inc/load_array_class.hpp"
#include "./classes.inc/load_non_array_class_by_bootstrap_class_loader.hpp"
#include "./classes.inc/load_non_array_class_by_user_class_loader.hpp"
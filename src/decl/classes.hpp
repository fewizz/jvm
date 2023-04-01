#pragma once

#include "./mutex_attribute_recursive.hpp"
#include "./class.hpp"
#include "./load_class.hpp"

#include <list.hpp>
#include <optional.hpp>

#include <posix/memory.hpp>

struct class_and_initiating_loaders {
	_class _class;
	// TODO make resizeable
	list<posix::memory_for_range_of<reference>> initiating_loaders;

	template<typename... Args>
	class_and_initiating_loaders(
		Args&&... args
	) :
		_class { forward<Args>(args)... },
		initiating_loaders {
			posix::allocate_memory_for<reference>(16)
		}
	{
		initiating_loaders.emplace_back(_class.defining_loader());
	}

	void record_as_initiating(reference ref) {
		if(loader_is_recorded_as_initiating(ref)) {
			print::err("class-loader is already recorded as initiating\n");
			posix::abort();
		}
		initiating_loaders.emplace_back(move(ref));
	}

	bool loader_is_recorded_as_initiating(reference ref) const {
		for(reference& l : initiating_loaders) {
			if(l.object_ptr() == ref.object_ptr()) {
				return true;
			}
		}
		return false;
	}
};

static struct classes :
	private list<posix::memory_for_range_of<class_and_initiating_loaders>>
{
	using base_type = list<
		posix::memory_for_range_of<class_and_initiating_loaders>
	>;
	using base_type::base_type;

	body<posix::mutex> mutex_ = posix::create_mutex(mutex_attribute_recursive);

	~classes() {
		for(class_and_initiating_loaders& c : *this) {
			c._class.destruct_declared_static_fields_values();
		}
	}

	body<posix::mutex>& mutex() { return mutex_; }

	template<basic_range Name>
	_class& find_or_load(Name&& name) {
		optional<_class&> c = try_find(name);
		return c.set_if_has_no_value([&]() -> _class& {
			return ::load_class_by_bootstrap_class_loader(
				name
			);
		}).get();
	}

	void mark_class_loader_as_initiating_for_class(_class& c, reference cl) {
		for(auto& c_and_cl : *this) {
			if(&c_and_cl._class == &c) {
				c_and_cl.record_as_initiating(cl);
				return;
			}
		}
		posix::abort();
	}

	template<basic_range Name>
	optional<class_and_initiating_loaders&>
	try_find_class_and_initiating_loaders(Name&& name) {
		optional<class_and_initiating_loaders&> res
			= this->try_find_first_satisfying(
				[&](class_and_initiating_loaders& c_and_l) {
					bool same_name = c_and_l._class.name()
						.has_equal_size_and_elements(name);

					return same_name;
				}
			);
		
		if(res.has_value()) {
			return { res.get() };
		}
		return {};
	}

	template<basic_range Name>
	optional<_class&> try_find_class_which_loading_was_initiated_by(
		Name&& name, reference class_loader
	) {
		optional<class_and_initiating_loaders&> possible_c_and_il
			= this->try_find_first_satisfying(
				[&](class_and_initiating_loaders& c_and_l) {
					bool same_name = c_and_l._class.name()
						.has_equal_size_and_elements(name);
					
					bool cl_is_initiating
						= c_and_l
						.loader_is_recorded_as_initiating(class_loader);

					return same_name && cl_is_initiating;
				}
			);
		
		if(possible_c_and_il.has_no_value()) {
			return {};
		}

		return possible_c_and_il.get()._class;
	}

	template<typename... Args>
	decltype(auto) emplace_back(Args&&... args) {
		return base_type::emplace_back(
			forward<Args>(args)...
		);
	}

} classes{ posix::allocate_memory_for<class_and_initiating_loaders>(65536) };
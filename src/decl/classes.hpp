#pragma once

#include "./mutex_attribute_recursive.hpp"
#include "./class.hpp"
#include "./class/load.hpp"

#include <list.hpp>
#include <optional.hpp>

#include <posix/memory.hpp>

static struct classes : private list<posix::memory_for_range_of<_class>> {
	using base_type = list<posix::memory_for_range_of<_class>>;
	using base_type::base_type;

	body<posix::mutex> mutex_
		= posix::create_mutex(get_mutex_attribute_recursive());

	template<basic_range Name>
	_class& find_or_load(Name&& name) {
		mutex_->lock();
		on_scope_exit unlock {[&] {
			mutex_->unlock();
		}};

		optional<_class&> c = try_find(name);
		return c.set_if_has_no_value([&]() -> _class& {
			return ::load_class(name);
		}).get();
	}

	template<basic_range Name>
	optional<_class&> try_find(Name&& name) {
		mutex_->lock();
		on_scope_exit unlock {[&] {
			mutex_->unlock();
		}};

		return this->try_find_first_satisfying(
			[&](_class& c) {
				return range{ c.name() }.has_equal_size_and_elements(name);
			}
		);
	}

	template<basic_range Name>
	_class& find_class(Name&& name) {
		mutex_->lock();
		on_scope_exit unlock {[&] {
			mutex_->unlock();
		}};

		return try_find(name).if_has_no_value([] {
			posix::abort();
		}).get();
	}

	template<basic_range Name>
	inline bool contains(Name&& name) {
		mutex_->lock();
		on_scope_exit unlock {[&] {
			mutex_->unlock();
		}};

		return try_find_class(name).has_value();
	}

	template<typename... Args>
	decltype(auto) emplace_back(Args&&... args) {
		mutex_->lock();
		on_scope_exit unlock {[&] {
			mutex_->unlock();
		}};

		return base_type::emplace_back(forward<Args>(args)...);
	}

} classes{ posix::allocate_memory_for<_class>(65536) };
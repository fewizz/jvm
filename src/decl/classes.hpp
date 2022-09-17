#pragma once

#include "class.hpp"
#include "class/load.hpp"

#include <list.hpp>
#include <optional.hpp>

#include <posix/memory.hpp>

static struct classes : list<posix::memory_for_range_of<_class>> {
	using base_type = list<posix::memory_for_range_of<_class>>;
	using base_type::base_type;

	template<basic_range Name>
	_class& find_or_load(Name&& name) {
		return try_find(name).set_if_has_no_value([&]() -> _class& {
			return ::load_class(name);
		}).value();
	}

	template<basic_range Name>
	optional<_class&> try_find(Name&& name) {
		return this->try_find_first_satisfying(
			[&](_class& c) {
				return range{ c.name() }.have_elements_equal_to(name);
			}
		);
	}

	template<basic_range Name>
	_class& find_class(Name&& name) {
		return try_find(name).if_has_no_value([] {
			abort();
		}).value();
	}

	template<basic_range Name>
	static inline bool contains(Name&& name) {
		return try_find_class(name).has_value();
	}

} classes{ posix::allocate_memory_for<_class>(65536) };
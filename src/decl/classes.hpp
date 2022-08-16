#pragma once

#include "class.hpp"
#include "class/load.hpp"

#include <memory_list.hpp>
#include <optional.hpp>

static struct classes : memory_list<_class, uint32> {
	using base_type = memory_list<_class, uint32>;
	using base_type::base_type;

	template<basic_range Name>
	_class& find_or_load(Name&& name) {
		return try_find(name).set_if_no_value([&]() -> _class& {
			return ::load_class(name);
		}).value();
	}

	template<basic_range Name>
	optional<_class&> try_find(Name&& name) {
		return range{ *this }.try_find_first_satisfying(
			[&](_class& c){ return range{ c.name() }.equals_to(name); }
		);
	}

	template<basic_range Name>
	_class& find_class(Name&& name) {
		return try_find(name).if_no_value([] {
			fprintf(stderr, "couldn't find class");
			abort();
		}).value();
	}

	template<basic_range Name>
	static inline bool contains(Name&& name) {
		return try_find_class(name).has_value();
	}

} classes{ allocate_for<_class>(65536) };
#pragma once

#include "./has_name_equal_to.hpp"

template<typename Type>
struct find_by_name_mixin {

	template<basic_range Name>
	auto try_find(Name&& name) {
		return range{ (Type&) *this }.try_find_last_satisfying(
			has_name_equal_to{ name }
		);
	}

	template<basic_range Name>
	auto try_find_index_of(Name&& name) {
		return range{ (Type&) *this }.try_find_index_of_last_satisfying(
			has_name_equal_to{ name }
		);
	}

	template<basic_range Name>
	decltype(auto) find(Name&& name) {
		return try_find(
			forward<Name>(name)
		).if_no_value([] { abort(); }).value();
	}

	template<basic_range Name>
	nuint find_index_of(Name&& name) {
		return try_find_index_of(
			forward<Name>(name)
		).if_no_value([] { abort(); }).value();
	}

};
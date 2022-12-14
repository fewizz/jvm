#pragma once

#include "./has_name_equal_to.hpp"
#include <posix/abort.hpp>

template<typename Type>
struct find_by_name_extension {
private:
	const Type& derived() const { return (const Type&) *this; }
	      Type& derived()       { return (      Type&) *this; }
public:

	template<basic_range Name>
	auto try_find(Name&& name) {
		return derived().try_find_last_satisfying(
			has_name_equal_to{ name }
		);
	}

	template<basic_range Name>
	auto try_find_index_of(Name&& name) {
		return derived().try_find_index_of_last_satisfying(
			has_name_equal_to{ name }
		);
	}

	template<basic_range Name>
	decltype(auto) find(Name&& name) {
		return try_find(
			forward<Name>(name)
		).if_no_value([] { abort(); }).get();
	}

	template<basic_range Name>
	nuint find_index_of(Name&& name) {
		return try_find_index_of(
			forward<Name>(name)
		).if_no_value([] { abort(); }).get();
	}

};
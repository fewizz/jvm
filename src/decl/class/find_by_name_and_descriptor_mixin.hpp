#pragma once

#include "./has_name_and_desriptor_equal_to.hpp"
#include "abort.hpp"

template<typename Type>
struct find_by_name_and_descriptor_mixin {

	template<basic_range Name, basic_range Descriptor>
	auto try_find(Name&& name, Descriptor&& descriptor) {
		return range{ (Type&) *this }.try_find_last_satisfying(
			has_name_and_desriptor_equal_to{ name, descriptor }
		);
	}

	template<basic_range Name, basic_range Descriptor>
	auto try_find_index_of(Name&& name, Descriptor&& descriptor) {
		return range{ (Type&) *this }.try_find_index_of_last_satisfying(
			has_name_and_desriptor_equal_to{ name, descriptor }
		);
	}

	template<basic_range Name, basic_range Descriptor>
	decltype(auto) find(Name&& name, Descriptor&& descriptor) {
		return try_find(
			forward<Name>(name), forward<Descriptor>(descriptor)
		).if_no_value([] { abort(); }).value();
	}

	template<basic_range Name, basic_range Descriptor>
	auto find_index_of(Name&& name, Descriptor&& descriptor) {
		return try_find_index_of(
			forward<Name>(name), forward<Descriptor>(descriptor)
		).if_no_value([] { abort(); }).value();
	}

};
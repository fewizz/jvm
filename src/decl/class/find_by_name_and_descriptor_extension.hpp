#pragma once

#include "./has_name_and_descriptor_equal_to.hpp"
#include <posix/abort.hpp>

template<typename Type, bool Dereference>
struct find_by_name_and_descriptor_extension {
private:
	decltype(auto) derived() const {
		if constexpr(Dereference) {
			return ((const Type&) *this).dereference_view();
		}
		else {
			return (const Type&) *this;
		}
	}
	decltype(auto) derived()       {
		if constexpr(Dereference) {
			return ((const Type&) *this).dereference_view();
		}
		else {
			return (const Type&) *this;
		}
	}
public:

	template<basic_range Name, basic_range Descriptor>
	auto try_find(Name&& name, Descriptor&& descriptor) {
		return derived().try_find_last_satisfying(
			has_name_and_descriptor_equal_to{ name, descriptor }
		);
	}

	template<basic_range Name, basic_range Descriptor>
	auto try_find_index_of(Name&& name, Descriptor&& descriptor) {
		return derived().try_find_index_of_last_satisfying(
			has_name_and_descriptor_equal_to{ name, descriptor }
		);
	}

	template<basic_range Name, basic_range Descriptor>
	decltype(auto) find(Name&& name, Descriptor&& descriptor) {
		return try_find(
			forward<Name>(name), forward<Descriptor>(descriptor)
		).if_has_no_value([] { abort(); }).value();
	}

	template<basic_range Name, basic_range Descriptor>
	auto find_index_of(Name&& name, Descriptor&& descriptor) {
		return try_find_index_of(
			forward<Name>(name), forward<Descriptor>(descriptor)
		).if_has_no_value([] { abort(); }).value();
	}

};
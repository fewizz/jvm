#pragma once

#include "../alloc.hpp"
#include "instance_field_index.hpp"
#include <core/meta/elements/one_of.hpp>
#include <core/fixed_vector.hpp>

struct _class;
struct method;
struct field;
struct static_field;

using trampoline_entry = elements::one_of<
	decltype(nullptr), _class&, method&, instance_field_index, static_field&
>;

struct trampoline_pool :
	protected fixed_vector<trampoline_entry, uint16, default_allocator>
{
	using base_type = fixed_vector<trampoline_entry, uint16, default_allocator>;
	using base_type::base_type;

	uint16 trampolines_count() const {
		return capacity();
	}

	const auto& trampoline(uint16 index) const { return (*this)[index - 1]; }
	auto& trampoline(uint16 index) { return (*this)[index - 1]; }

};
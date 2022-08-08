#pragma once

#include "method/with_class.hpp"

#include <elements/one_of.hpp>
#include <elements/none.hpp>

struct _class;
struct static_field;

using trampoline_entry = elements::one_of<
	elements::none, _class&, method_with_class,
	static_field_with_class,
	reference
>;

struct trampoline_pool :
	protected memory_list<trampoline_entry, uint16>
{
	using base_type = memory_list<trampoline_entry, uint16>;

	trampoline_pool(memory_span memory_span) : base_type{ memory_span } {
		base_type::fill(elements::none{});
	}

	using base_type::emplace_back;

	uint16 trampolines_count() const {
		return capacity();
	}

	const auto& trampoline(uint16 index) const { return (*this)[index - 1]; }
	auto& trampoline(uint16 index) { return (*this)[index - 1]; }

};
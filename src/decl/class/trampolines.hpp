#pragma once

#include <elements/one_of.hpp>
#include <elements/none.hpp>

#include <memory_list.hpp>

struct static_field;

using trampoline = elements::one_of<
	elements::none
>;

struct trampolines :
	protected memory_list<trampoline, uint16>
{
	using base_type = memory_list<trampoline, uint16>;

	trampolines(memory_span memory_span) : base_type{ memory_span } {
		base_type::fill(elements::none{});
	}

	using base_type::emplace_back;

	uint16 trampolines_count() const {
		return capacity();
	}

	const ::trampoline& trampoline(uint16 index) const {
		return (*this)[index - 1];
	}

	      ::trampoline& trampoline(uint16 index)       {
		return (*this)[index - 1];
	}

};
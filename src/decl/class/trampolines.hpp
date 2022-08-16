#pragma once

#include "object/reference.hpp"
#include "method.hpp"
#include "field/value.hpp"
#include "class/instance_methods.hpp"
#include "class/instance_fields.hpp"
#include "alloc.hpp"

#include <elements/one_of.hpp>
#include <elements/none.hpp>

#include <memory_list.hpp>

struct static_field;

using trampoline = elements::one_of<
	elements::none, reference, _class&, method&, field_value,
	instance_method_index, instance_field_index
>;

struct trampolines :
	protected memory_list<trampoline, uint16>
{
	using base_type = memory_list<trampoline, uint16>;

	trampolines(uint16 count) : base_type{ allocate_for<::trampoline>(count) } {
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
#pragma once

#include "object/reference.hpp"
#include "method.hpp"
#include "field/value.hpp"
#include "class/instance_methods.hpp"
#include "class/instance_fields.hpp"

#include <optional.hpp>
#include <list.hpp>

#include <posix/memory.hpp>

struct static_field;

using trampoline = optional<
	reference, _class&, method&, field_value,
	instance_method_index, instance_field_index
>;

struct trampolines :
	protected list<posix::memory_for_range_of<trampoline>>
{
	using base_type = list<posix::memory_for_range_of<trampoline>>;

	trampolines(uint16 count) :
		base_type{ posix::allocate_memory_for<::trampoline>(count) }
	{
		base_type::fill_with(::trampoline{});
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
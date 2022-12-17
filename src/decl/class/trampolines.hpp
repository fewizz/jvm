#pragma once

#include "reference.hpp"
#include "method.hpp"
#include "field/value.hpp"
#include "class/member_index.hpp"

#include <optional.hpp>

#include <posix/memory.hpp>

struct static_field;

using trampoline = optional<
	reference, _class&, method&, field_value&,
	instance_method_index, instance_field_index,
	tuple<instance_field_index, _class&>
>;

struct trampolines :
	protected posix::memory_for_range_of<trampoline>
{
	using base_type = posix::memory_for_range_of<trampoline>;

	trampolines(uint16 count) :
		base_type{ posix::allocate_memory_for<::trampoline>(count) }
	{
		for(storage<::trampoline>& s : *this) {
			s.construct(::trampoline{});
		}
	}

	using base_type::size;

	const ::trampoline& trampoline(uint16 index) const {
		return base_type::as_span()[index - 1];
	}

	      ::trampoline& trampoline(uint16 index)       {
		return base_type::as_span()[index - 1];
	}

};
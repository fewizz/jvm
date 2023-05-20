#pragma once

#include "./layout.hpp"
#include "./member_index.hpp"

#include "reference.hpp"
#include "method.hpp"

#include <optional.hpp>

#include <posix/memory.hpp>

using trampoline = optional<
	reference, c&, method&, field&,
	instance_method_index, instance_field_index
>;

struct trampolines :
	protected posix::memory<trampoline>
{
	using base_type = posix::memory<trampoline>;

	trampolines(uint16 count) :
		base_type {
			posix::allocate<::trampoline>(count)
		}
	{
		for(storage<::trampoline>& s : *this) {
			s.construct<::trampoline>(::trampoline{});
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
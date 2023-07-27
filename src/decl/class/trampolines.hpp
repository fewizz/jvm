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
	protected initialised<posix::memory<trampoline>>
{
	using base_type = initialised<posix::memory<trampoline>>;

	trampolines(uint16 count) :
		base_type ( [&] {
			auto uninitialised = posix::allocate<::trampoline>(count);
			for(storage<::trampoline>& s : uninitialised) {
				s.construct<::trampoline>(::trampoline{});
			}
			return initialised{ move(uninitialised) };
		}() )
	{}

	using base_type::size;

	const ::trampoline& trampoline(uint16 index) const {
		return base_type::operator [] (index - 1);
	}

	      ::trampoline& trampoline(uint16 index)       {
		return base_type::operator [] (index - 1);
	}

	void reset() {
		for(::trampoline& trampoline : *this) {
			trampoline.reset();
		}
	}

};
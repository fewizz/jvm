#pragma once

#include "./layout.hpp"
#include "./member_index.hpp"

#include "reference.hpp"
#include "method.hpp"

#include <optional.hpp>

#include <posix/memory.hpp>

struct static_field;

struct class_and_declared_static_field_index {
	_class& _class;
	declared_static_field_index field_index;
};

struct instance_field_index_and_stack_size {
	instance_field_index field_index;
	uint8 stack_size;
};

using trampoline = optional<
	class_and_declared_static_field_index,
	instance_field_index_and_stack_size,
	reference, _class&, method&,
	instance_method_index, instance_field_index
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
#pragma once

#include "execution/stack_entry.hpp"
#include "execute.hpp"

template<typename Descriptor>
static optional<stack_entry> native_interface_call(
	native_function_ptr ptr, arguments_span args, Descriptor&&
);
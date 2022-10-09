#pragma once

#include "execution/stack_entry.hpp"
#include "execute.hpp"

static optional<stack_entry> native_interface_call(
	native_function_ptr ptr, method& m
);
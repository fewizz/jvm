#pragma once

#include "stack_entry.hpp"
#include "../method.hpp"

inline stack_entry execute(
	method& m, span<stack_entry, uint16> args = {}
);
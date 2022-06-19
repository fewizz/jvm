#pragma once

#include "stack_entry.hpp"
#include "../method/with_class/declaration.hpp"

inline stack_entry execute(
	method_with_class m, span<stack_entry, uint16> args = {}
);
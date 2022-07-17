#pragma once

#include "stack_entry.hpp"
#include "../method/with_class/decl.hpp"

static stack_entry execute(
	method_with_class m, span<stack_entry, uint16> args = {}
);
#pragma once

#include "execution/stack_entry.hpp"
#include "method/with_class/decl.hpp"

using arguments_container = span<stack_entry, uint16>;

static stack_entry invoke(
	method_with_class m, arguments_container args = {}
);
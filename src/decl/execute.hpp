#pragma once

#include "execution/stack_entry.hpp"
#include "method/with_class.hpp"

using arguments_span = span<stack_entry, uint16>;

static stack_entry execute(
	method_with_class m, arguments_span args = {}
);
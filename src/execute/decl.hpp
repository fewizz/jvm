#pragma once

#include "./stack_entry.hpp"

#include "method/with_class/decl.hpp"

#include <core/expected.hpp>

static expected<stack_entry, reference> execute(
	method_with_class m, span<stack_entry, uint16> args = {}
);
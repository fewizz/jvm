#pragma once

#include "execution/stack_entry.hpp"
#include "method.hpp"

using arguments_span = span<stack_entry, uint16>;

static optional<stack_entry> execute(method& m, arguments_span args = {});
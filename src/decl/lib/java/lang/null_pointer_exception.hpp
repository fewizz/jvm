#pragma once

#include "class.hpp"
#include "method.hpp"
#include "reference.hpp"

static optional<_class&> null_pointer_exception_class;
static optional<method&> null_pointer_exception_constructor;

[[nodiscard]] inline expected<reference, reference>
try_create_null_pointer_exception();
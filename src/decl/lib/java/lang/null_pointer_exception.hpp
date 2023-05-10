#pragma once

#include "class.hpp"
#include "method.hpp"
#include "reference.hpp"

static optional<c&> null_pointer_exception_class;
static optional<instance_method&> null_pointer_exception_constructor;

[[nodiscard]] inline expected<reference, reference>
try_create_null_pointer_exception();
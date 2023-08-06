#pragma once

#include "class.hpp"
#include "reference.hpp"

inline c& get_stack_overflow_error_class();

[[nodiscard]] inline expected<reference, reference>
try_create_stack_overflow_error();
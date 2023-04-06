#pragma once

#include "decl/reference.hpp"

[[nodiscard]] inline expected<reference, reference>
try_create_class_cast_exception();
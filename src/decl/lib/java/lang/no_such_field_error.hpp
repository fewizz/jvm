#pragma once

#include "reference.hpp"

#include <expected.hpp>

[[nodiscard]] inline expected<reference, reference>
try_create_no_such_field_error();
#pragma once

#include "reference.hpp"

#include <expected.hpp>

[[nodiscard]] inline expected<reference, reference>
try_create_incompatible_class_change_error();
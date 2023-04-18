#pragma once

#include "class.hpp"

#include <optional.hpp>

static optional<_class&> class_not_found_exception_class;

[[nodiscard]] inline expected<reference, reference>
try_create_class_not_found_exception();

[[nodiscard]] inline expected<reference, reference>
try_create_class_not_found_exception(reference message);
#pragma once

#include "reference.hpp"

#include <expected.hpp>

[[nodiscard]] inline expected<reference, reference>
try_create_abstract_method_error();
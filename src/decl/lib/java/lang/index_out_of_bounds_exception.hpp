#pragma once

#include "decl/class.hpp"
#include "decl/method.hpp"
#include "decl/reference.hpp"

#include <optional.hpp>

static optional<c&> index_of_of_bounds_exception_class;
static optional<method&> index_of_of_bounds_exception_constructor;

[[nodiscard]] inline expected<reference, reference>
try_create_index_of_of_bounds_exception();
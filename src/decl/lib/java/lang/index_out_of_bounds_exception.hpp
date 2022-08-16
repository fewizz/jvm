#pragma once

#include "decl/class.hpp"
#include "decl/method.hpp"
#include "decl/object/reference.hpp"

#include <optional.hpp>

static optional<_class&> index_of_of_bounds_exception_class;
static optional<method&> index_of_of_bounds_exception_constructor;

reference create_index_of_of_bounds_exception();
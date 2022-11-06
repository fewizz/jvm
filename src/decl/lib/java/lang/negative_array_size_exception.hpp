#pragma once

#pragma once

#include "class.hpp"
#include "method.hpp"
#include "reference.hpp"

#include <optional.hpp>
#include <span.hpp>

static optional<_class&> negative_array_size_exception_class;
static optional<method&> negative_array_size_exception_constructor;

static reference create_negative_array_size_exception();
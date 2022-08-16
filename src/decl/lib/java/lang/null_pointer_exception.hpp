#pragma once

#include "class.hpp"
#include "method.hpp"
#include "object/reference.hpp"

static optional<_class&> null_pointer_exception_class;
static optional<method&> null_pointer_exception_constructor;

inline reference create_null_pointer_exception();
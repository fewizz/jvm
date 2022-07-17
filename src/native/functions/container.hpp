#pragma once

#include "../function/decl.hpp"
#include "../../alloc.hpp"
#include <core/limited_list.hpp>

static inline
limited_list<native_function, uint16, default_allocator>
native_functions{ 65535 };
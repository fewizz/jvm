#pragma once

#include "class/declaration.hpp"
#include "../alloc.hpp"

#include <core/limited_list.hpp>

// TODO temporary
static limited_list<_class&, nuint, default_allocator> callers{ 65536 };
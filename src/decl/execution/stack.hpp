#pragma once

#include "stack_entry.hpp"
#include "alloc.hpp"

#include <core/limited_list.hpp>

struct stack : limited_list<stack_entry, uint16, default_allocator> {
	using base_type = limited_list<stack_entry, uint16, default_allocator>;
	using base_type::base_type;
};
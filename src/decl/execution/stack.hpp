#pragma once

#include "./stack_entry.hpp"

#include <memory_list.hpp>

struct stack : memory_list<stack_entry, uint16> {
	using base_type = memory_list<stack_entry, uint16>;
	using base_type::base_type;
};
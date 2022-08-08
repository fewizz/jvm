#pragma once

#include "method.hpp"

#include <memory_list.hpp>

struct declared_methods : memory_list<method, uint16> {
	using base_type = memory_list<method, uint16>;
	using base_type::base_type;
};
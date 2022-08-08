#pragma once

#include "field.hpp"

#include <memory_list.hpp>

struct declared_fields : memory_list<field, uint16> {
	using base_type = memory_list<field, uint16>;
	using base_type::base_type;
};
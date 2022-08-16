#pragma once

#include "field.hpp"
#include "./field_index.hpp"

#include <memory_list.hpp>

struct declared_fields : memory_list<field, declared_field_index> {
	using base_type = memory_list<field, declared_field_index>;
	using base_type::base_type;
};
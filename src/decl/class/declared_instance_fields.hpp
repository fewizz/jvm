#pragma once

#include "./find_by_name_and_descriptor_mixin.hpp"
#include "./field_index.hpp"
#include "field.hpp"

#include <memory_list.hpp>

struct declared_instance_fields :
	memory_list<field&, declared_field_index>,
	find_by_name_and_descriptor_mixin<::declared_instance_fields>
{
	using base_type = memory_list<field&, declared_field_index>;
	using base_type::base_type;
};
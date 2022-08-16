#pragma once

#include <memory_list.hpp>

struct _class;

struct declared_interfaces : memory_list<_class&, uint16> {
	using base_type = memory_list<_class&, uint16>;
	using base_type::base_type;
};
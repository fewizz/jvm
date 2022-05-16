#pragma once

#include <core/integer.hpp>

struct instance_field_index {
	uint16 _;
	operator uint16 () const { return _; }
};
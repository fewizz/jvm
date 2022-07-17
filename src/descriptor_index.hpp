#pragma once

#include <core/integer.hpp>

struct descriptor_index {
	uint16 value_;
	operator uint16 () const { return value_; }
};
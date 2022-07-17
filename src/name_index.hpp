#pragma once

#include <core/integer.hpp>

struct name_index {
	uint16 value_;
	operator uint16 () const { return value_; }
};
#pragma once

#include <core/integer.hpp>

class field_index {
	uint16 value_;
public:

	field_index(uint16 value) : value_{ value } {}

	operator const uint16& () const & { return value_; }
	operator       uint16& ()       & { return value_; }

};
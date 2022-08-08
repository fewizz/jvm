#pragma once

#include <integer.hpp>

class parameters_count {
	uint8 value_;
public:

	parameters_count(uint8 value) : value_{ value } {}

	operator const uint8& () const & { return value_; }
	operator       uint8& () &       { return value_; }

};
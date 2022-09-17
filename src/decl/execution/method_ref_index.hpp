#pragma once

#include <integer.hpp>

class method_ref_index {
	uint16 value_;
public:

	explicit method_ref_index(uint16 value_) : value_(value_) {}

	operator uint16& () { return value_; }

};
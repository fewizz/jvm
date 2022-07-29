#pragma once

#include <core/integer.hpp>

class name_index {
	uint16 value_;
public:

	name_index(uint16 value) : value_{ value } {}

	operator uint16& () { return value_; }

};
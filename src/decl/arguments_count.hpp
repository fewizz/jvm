#pragma once

#include <core/integer.hpp>

class arguments_count {
	uint8 value_;
public:

	explicit arguments_count(uint8 value) : value_{ value } {}

	operator uint8& () { return value_; }

};
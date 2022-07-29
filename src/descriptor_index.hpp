#pragma once

#include <core/integer.hpp>

class descriptor_index {
	uint16 value_;
public:

	explicit descriptor_index(uint16 value) : value_{ value } {}

	operator uint16& () { return value_; }

};
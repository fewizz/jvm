#pragma once

#include <integer.hpp>

class instance_field_position {
	nuint value_;
public:
	explicit constexpr instance_field_position() = default;
	explicit constexpr instance_field_position(nuint value) : value_{ value } {}
	operator nuint () const { return value_; }
};
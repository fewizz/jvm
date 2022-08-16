#pragma once

#include <integer.hpp>

class field_index {
	uint16 value_;
public:

	field_index() = default;
	field_index(uint16 value) : value_{ value } {}

	operator const uint16& () const & { return value_; }
	operator       uint16& ()       & { return value_; }
};

struct declared_field_index : field_index {
	using field_index::field_index;
};

struct declared_instance_field_index : field_index {
	using field_index::field_index;
};

struct instance_field_index : field_index {
	using field_index::field_index;
};

struct declared_static_field_index : field_index {
	using field_index::field_index;
};
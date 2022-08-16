#pragma once

#include <integer.hpp>

class method_index {
	uint16 value_;
public:

	method_index() = default;
	method_index(uint16 value) : value_{ value } {}

	operator const uint16& () const & { return value_; }
	operator       uint16& ()       & { return value_; }
};

struct declared_method_index : method_index {
	using method_index::method_index;
};

struct declared_instance_method_index : method_index {
	using method_index::method_index;
};

struct instance_method_index : method_index {
	using method_index::method_index;
};

struct declared_static_method_index : method_index {
	using method_index::method_index;
};
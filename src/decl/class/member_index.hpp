#pragma once

#include <integer.hpp>

class class_member_index {
	uint16 value_;
public:
	class_member_index() = default;
	explicit class_member_index(uint16 value) : value_{ value } {}
	explicit operator uint16 () const { return value_; }
};

struct method_member_index : class_member_index {
	using class_member_index::class_member_index;
};

struct declared_instance_method_index : method_member_index {
	using method_member_index::method_member_index;
};

struct declared_static_method_index : method_member_index {
	using method_member_index::method_member_index;
};

struct instance_method_index : method_member_index {
	using method_member_index::method_member_index;
};

struct field_member_index : class_member_index {
	using class_member_index::class_member_index;
};

struct declared_instance_field_index : field_member_index {
	using field_member_index::field_member_index;
};

struct declared_static_field_index : field_member_index {
	using field_member_index::field_member_index;
};

struct instance_field_index : field_member_index {
	using field_member_index::field_member_index;
};
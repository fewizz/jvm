#pragma once

#include <integer.hpp>

class declared_field_index {
	uint16 value_;
public:
	declared_field_index(uint16 value) : value_{ value } {}
	operator uint16 () const { return value_; }
};

class declared_method_index {
	uint16 value_;
public:
	declared_method_index(uint16 value) : value_{ value } {}
	operator uint16 () const { return value_; }
};

class instance_method_index {
	uint16 value_;
public:
	instance_method_index(uint16 value) : value_{ value } {}
	operator uint16 () const { return value_; }
};

class instance_field_index {
	uint16 value_;
public:
	instance_field_index(uint16 value) : value_{ value } {}
	operator uint16 () const { return value_; }
};

class declared_static_field_index {
	uint16 value_;
public:
	declared_static_field_index(uint16 value) : value_{ value } {}
	operator uint16 () const { return value_; }
};
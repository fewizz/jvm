#pragma once

class is_primitive_class {
	bool value_;
public:
	is_primitive_class(bool value): value_{ value } {}
	operator       bool& ()       & { return value_; }
	operator const bool& () const & { return value_; }
};
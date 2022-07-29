#pragma once

#include <core/integer.hpp>

template<unsigned_integer Type = uint8>
class constant_index_of {
	Type value_;
public:
	explicit constant_index_of(Type value) : value_{ value } {}
	operator Type& () { return value_; }
};

using constant_index = constant_index_of<uint8>;
using wide_constant_index = constant_index_of<uint16>;
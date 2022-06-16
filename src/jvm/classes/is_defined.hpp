#pragma once

#include "find.hpp"

template<range Name>
static inline bool class_is_defined(Name name) {
	return try_find_class(name).has_value();
}
#pragma once

#include "find_or_load.hpp"

template<range Name>
static inline _class& find_or_load_class(Name name) {
	if(optional<_class&> c = try_find_class(name); c.has_value()) {
		return c.value();
	}
	return load_class(name);
}
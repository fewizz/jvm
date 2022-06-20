#pragma once

#include "reference/declaration.hpp"
#include "declaration.hpp"

inline reference create_object(_class& c) {
	object* o = new(malloc(sizeof(object))) object(c);
	return { *o };
}
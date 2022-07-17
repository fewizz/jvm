#pragma once

#include "reference/decl.hpp"
#include "decl.hpp"

inline reference create_object(_class& c) {
	object* o = new(malloc(sizeof(object))) object(c);
	return { *o };
}
#pragma once

#include "reference.hpp"
#include "declaration.hpp"

inline reference create_object(_class& c) {
	object* o = new(malloc(sizeof(object))) object(c);
	o->on_reference_added(); // references() == 1
	return { *o };
}
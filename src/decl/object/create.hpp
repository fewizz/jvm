#pragma once

#include "reference.hpp"
#include "class.hpp"
#include "object.hpp"

inline reference create_object(_class& c) {
	c.initialise_if_need();
	object* o = new(malloc(sizeof(object))) object(c);
	return { *o };
}
#pragma once

#include "reference/decl.hpp"
#include "class/decl.hpp"
#include "./decl.hpp"

inline reference create_object(_class& c) {
	c.initialise_if_need();
	object* o = new(malloc(sizeof(object))) object(c);
	return { *o };
}
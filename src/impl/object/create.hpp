#include "decl/object/create.hpp"
#include "decl/object.hpp"
#include "decl/class.hpp"

inline reference create_object(_class& c) {
	c.initialise_if_need();
	object* o = new(malloc(sizeof(object))) object(c);
	return { *o };
}
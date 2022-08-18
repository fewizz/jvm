#include "decl/object/create.hpp"
#include "decl/object.hpp"
#include "decl/class.hpp"
#include "decl/alloc.hpp"

inline reference create_object(_class& c) {
	c.initialise_if_need();
	object* o = new(allocate_for<object>(1).elements_ptr()) object(c);
	return { *o };
}
#include "decl/object/create.hpp"
#include "decl/object.hpp"
#include "decl/class.hpp"

#include <posix/memory.hpp>

inline reference create_object(_class& c) {
	c.initialise_if_need();
	object* ptr = posix::allocate_raw_memory_of<object>(1).iterator();
	new(ptr) object(c);
	return { *ptr };
}
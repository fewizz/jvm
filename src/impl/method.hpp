#include "decl/method.hpp"
#include "decl/class.hpp"

/* "A method is an instance initialization method
if all of the following are true:" */
inline bool method::is_instance_initialisation() const {
	return
		/* "It is defined in a class (not an interface)." */
		!_class().is_interface() &&
		/* "It has the special name <init>." */
		range{ name() }.equals_to(c_string{ "<init>" }) &&
		/* "It is void (§4.3.3)." */
		is_void();
}
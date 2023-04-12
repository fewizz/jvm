#include "decl/method.hpp"

#include "decl/class.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/invoke/var_handle.hpp"

/* "A method is an instance initialization method
    if all of the following are true:" */
inline bool method::is_instance_initialisation() const {
	return
		/* "It is defined in a class (not an interface)." */
		!_class().is_interface() &&
		/* "It has the special name <init>." */
		name().has_equal_size_and_elements(c_string{ "<init>" }) &&
		/* "It is void (§4.3.3)." */
		is_void();
}

inline bool method::is_signature_polymorphic() const {
	const ::_class& c = _class();

	bool c_is_mh_or_vh =
		&c == method_handle_class.ptr() ||
		&c == var_handle_class.ptr();
	
	bool param_is_object_array =
		descriptor().starts_with(c_string{"(([Ljava/lang/Object;))"});
	
	bool varargs_and_native_flags_set =
		access_flags().varargs &&
		access_flags().native;

	return
		c_is_mh_or_vh &&
		param_is_object_array &&
		varargs_and_native_flags_set;
}
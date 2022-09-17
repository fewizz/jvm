#include "decl/lib/java/lang/object.hpp"

#include "decl/class.hpp"
#include "decl/class/load.hpp"
#include "decl/object.hpp"
#include "decl/native/interface/environment.hpp"

static inline void init_java_lang_object() {
	object_class = load_class(c_string{ "java/lang/Object" });

	object_class->declared_methods()
	.find(c_string{ "hashCode" }, c_string{ "()I" })
	->native_function(
		(void*) (int32(*)(native_interface_environment*, object*))
		[](native_interface_environment*, object* o) {
			return (int32) (nuint) o;
		}
	);

	object_class->declared_methods()
	.find(c_string{ "getClass" }, c_string{ "()Ljava/lang/Class;" })
	->native_function(
		(void*) (object*(*)(native_interface_environment*, object*))
		[](native_interface_environment*, object* o) -> object* {
			return o->_class().instance().object_ptr();
		}
	);

}
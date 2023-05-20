#include "decl/lib/java/lang/object.hpp"

#include "decl/class.hpp"
#include "decl/classes.hpp"
#include "decl/object.hpp"
#include "decl/native/environment.hpp"

static inline void init_java_lang_object() {
	object_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/Object" }
	);

	object_class->declared_instance_methods()
	.find(c_string{ u8"hashCode" }, c_string{ u8"()I" })
	.native_function(
		(void*)+[](native_environment*, object* o) {
			return (int32) (nuint) o;
		}
	);

	object_class->declared_instance_methods()
	.find(c_string{ u8"getClass" }, c_string{ u8"()Ljava/lang/Class;" })
	.native_function(
		(void*)+[](native_environment*, object* o) -> ::object* {
			return o->c().object_ptr();
		}
	);

}
#include "decl/classes.hpp"
#include "decl/native/interface/environment.hpp"

static void init_java_lang_void() {
	classes.find_or_load(c_string{ "java/lang/Void" })
	.declared_methods().find(
		c_string{ "getPrimitiveClass" }, c_string{ "()Ljava/lang/Class;" }
	).native_function(
		(void*) (object*(*)(native_interface_environment*))
		[](native_interface_environment*) -> object* {
			return void_class->instance().object_ptr();
		}
	);
}
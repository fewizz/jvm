#include "decl/classes.hpp"

static void init_java_lang_void() {
	classes.find_or_load(c_string{ "java/lang/Void" })
	.declared_methods().find(
		c_string{ "getPrimitiveClass" }, c_string{ "()Ljava/lang/Class;" }
	)->native_function(
		(void*) (object*(*)()) []() -> object* {
			return void_class->instance().object_ptr();
		}
	);
}
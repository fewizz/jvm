#include "decl/native/functions.hpp"
#include "decl/primitives.hpp"

static void init_java_lang_integer() {

	native_functions.emplace_back(
		(void*) (object*(*)()) []() -> object* {
			return int_class->instance().object_ptr();
		},
		c_string{ "Java_java_lang_Integer_getPrimitiveClass" },
		c_string{ "()Ljava/lang/Class;" }
	);

}
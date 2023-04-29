#include "decl/classes.hpp"
#include "decl/primitives.hpp"
#include "decl/native/environment.hpp"

static void init_java_lang_void() {
	c& c = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/Void" }
	);

	c.declared_static_methods().find(
		c_string{ "getPrimitiveClass" }, c_string{ "()Ljava/lang/Class;" }
	).native_function(
		(void*)+[](native_environment*) -> object* {
			return void_class->object_ptr();
		}
	);
}
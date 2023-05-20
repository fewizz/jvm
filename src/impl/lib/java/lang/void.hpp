#include "decl/classes.hpp"
#include "decl/primitives.hpp"
#include "decl/native/environment.hpp"

static void init_java_lang_void() {
	c& c = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/Void" }
	);

	c.declared_static_methods().find(
		c_string{ u8"getPrimitiveClass" }, c_string{ u8"()Ljava/lang/Class;" }
	).native_function(
		(void*)+[](native_environment*) -> o<jl::object>* {
			return void_class->object_ptr();
		}
	);
}
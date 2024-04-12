#include "decl/classes.hpp"
#include "decl/primitives.hpp"
#include "decl/native/environment.hpp"

static void init_java_lang_void() {
	c& c = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/Void"s
	);

	c.declared_static_methods().find(
		u8"getPrimitiveClass"s, u8"()Ljava/lang/Class;"s
	).native_function(
		(void*)+[](native_environment*) -> object* {
			return void_class->object_ptr();
		}
	);
}
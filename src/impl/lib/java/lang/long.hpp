#include "decl/lib/java/lang/long.hpp"

#include "decl/classes.hpp"
#include "decl/primitives.hpp"

inline void init_java_lang_long() {
	java_lang_long_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/Long" }
	);

	java_lang_long_constructor =
		java_lang_long_class->declared_instance_methods()
		.find(c_string{ u8"<init>" }, c_string{ u8"(J)V" });

	java_lang_long_value_field_position =
		java_lang_long_class->instance_field_position(
			c_string{ u8"value_" }, c_string{ u8"J" }
		);

	java_lang_long_class->declared_static_methods().find(
		c_string{ u8"getPrimitiveClass" }, c_string{ u8"()Ljava/lang/Class;" }
	).native_function(
		(void*)+[]() -> o<jl::object>* {
			return long_class->object_ptr();
		}
	);
}
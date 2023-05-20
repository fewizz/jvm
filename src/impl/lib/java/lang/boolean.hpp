#include "decl/lib/java/lang/boolean.hpp"

#include "decl/native/environment.hpp"

#include "classes.hpp"

static void init_java_lang_boolean() {
	java_lang_boolean_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/Boolean" }
	);

	java_lang_boolean_constructor =
		java_lang_boolean_class->declared_instance_methods()
		.find(c_string{ u8"<init>" }, c_string{ u8"(Z)V" });

	java_lang_boolean_value_field_position =
		java_lang_boolean_class->instance_field_position(
			c_string{ u8"value_" }, c_string{ u8"Z" }
		);

	java_lang_boolean_class->declared_static_methods().find(
		c_string{ u8"getPrimitiveClass" }, c_string{ u8"()Ljava/lang/Class;" }
	).native_function((void*)+[](native_environment*) -> o<jl::object>* {
		return bool_class->object_ptr();
	});
}
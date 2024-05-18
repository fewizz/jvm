#include "decl/lib/java/lang/boolean.hpp"

#include "decl/native/environment.hpp"

#include "classes.hpp"

static void init_java_lang_boolean() {
	java_lang_boolean_class = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/Boolean"sv
	);

	java_lang_boolean_constructor =
		java_lang_boolean_class->declared_instance_methods()
		.find(u8"<init>"sv, u8"(Z)V"sv);

	java_lang_boolean_value_field_position =
		java_lang_boolean_class->instance_field_position(
			u8"value_"sv, u8"Z"sv
		);

	java_lang_boolean_class->declared_static_methods().find(
		u8"getPrimitiveClass"sv, u8"()Ljava/lang/Class;"sv
	).native_function((void*)+[](native_environment*) -> object* {
		return bool_class->object_ptr();
	});
}
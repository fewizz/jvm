#include "decl/lib/java/lang/short.hpp"

#include "decl/native/environment.hpp"

#include "classes.hpp"

static void init_java_lang_short() {
	java_lang_short_class = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/Short"sv
	);

	java_lang_short_constructor =
		java_lang_short_class->declared_instance_methods()
		.find(u8"<init>"sv, u8"(S)V"sv);
	
	java_lang_short_value_field_position =
		java_lang_short_class->instance_field_position(
			u8"value_"sv, u8"S"sv
		);

	java_lang_short_class->declared_static_methods().find(
		u8"getPrimitiveClass"sv, u8"()Ljava/lang/Class;"sv
	).native_function((void*)+[](native_environment*) -> object* {
		return short_class->object_ptr();
	});
}
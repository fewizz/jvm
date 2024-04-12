#include "decl/lib/java/lang/byte.hpp"

#include "decl/native/environment.hpp"

#include "classes.hpp"

static void init_java_lang_byte() {
	java_lang_byte_class = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/Byte"s
	);

	java_lang_byte_constructor =
		java_lang_byte_class->declared_instance_methods()
		.find(u8"<init>"s, u8"(B)V"s);
	
	java_lang_byte_value_field_position =
		java_lang_byte_class->instance_field_position(
			u8"value_"s, u8"B"s
		);

	java_lang_byte_class->declared_static_methods().find(
		u8"getPrimitiveClass"s, u8"()Ljava/lang/Class;"s
	).native_function((void*)+[](native_environment*) -> object* {
		return byte_class->object_ptr();
	});
}
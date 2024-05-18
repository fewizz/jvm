#include "decl/lib/java/lang/character.hpp"

#include "decl/native/environment.hpp"
#include "decl/primitives.hpp"

#include "classes.hpp"

static void init_java_lang_character() {
	java_lang_character_class = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/Character"sv
	);

	java_lang_character_constructor =
		java_lang_character_class->declared_instance_methods()
		.find(u8"<init>"sv, u8"(C)V"sv);
	
	java_lang_character_value_field_position =
		java_lang_character_class->instance_field_position(
			u8"value_"sv, u8"C"sv
		);

	java_lang_character_class->declared_static_methods().find(
		u8"getPrimitiveClass"sv, u8"()Ljava/lang/Class;"sv
	).native_function((void*)+[](native_environment*) -> object* {
		return char_class->object_ptr();
	});
}
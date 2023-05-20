#include "decl/lib/java/lang/character.hpp"

#include "decl/native/environment.hpp"
#include "decl/primitives.hpp"

#include "classes.hpp"

static void init_java_lang_character() {
	java_lang_character_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/Character" }
	);

	java_lang_character_constructor =
		java_lang_character_class->declared_instance_methods()
		.find(c_string{ u8"<init>" }, c_string{ u8"(C)V" });
	
	java_lang_character_value_field_position =
		java_lang_character_class->instance_field_position(
			c_string{ u8"value_" }, c_string{ u8"C" }
		);

	java_lang_character_class->declared_static_methods().find(
		c_string{ u8"getPrimitiveClass" }, c_string{ u8"()Ljava/lang/Class;" }
	).native_function((void*)+[](native_environment*) -> o<jl::object>* {
		return char_class->object_ptr();
	});
}
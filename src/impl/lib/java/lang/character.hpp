#include "decl/lib/java/lang/character.hpp"

#include "classes.hpp"

static void init_java_lang_character() {
	java_lang_character_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/Character" }
	);

	java_lang_character_constructor =
		java_lang_character_class->declared_instance_methods()
		.find(c_string{"<init>"}, c_string{"(C)V"});
	
	java_lang_character_value_field_position =
		java_lang_character_class->instance_field_position(
			c_string{"value_", }, c_string{"C"}
		);
}
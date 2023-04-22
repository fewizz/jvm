#include "decl/lib/java/lang/short.hpp"

#include "decl/native/environment.hpp"

#include "classes.hpp"

static void init_java_lang_short() {
	java_lang_short_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/Short" }
	);

	java_lang_short_constructor =
		java_lang_short_class->declared_instance_methods()
		.find(c_string{"<init>"}, c_string{"(S)V"});
	
	java_lang_short_value_field_position =
		java_lang_short_class->instance_field_position(
			c_string{"value_", }, c_string{"S"}
		);

	java_lang_short_class->declared_static_methods().find(
		c_string{"getPrimitiveClass"}, c_string{"()Ljava/lang/Class;"}
	).native_function((void*)+[](native_environment*) -> object* {
		return short_class->instance().object_ptr();
	});
}
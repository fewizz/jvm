#include "decl/lib/java/lang/long.hpp"

#include "decl/classes.hpp"
#include "decl/primitives_classes.hpp"

inline void init_java_lang_long() {
	java_lang_long_class = classes.load_class_by_bootstrap_class_loader(
		c_string{"java/lang/Long"}
	);

	java_lang_long_constructor =
		java_lang_long_class->declared_instance_methods()
		.find(c_string{"<init>"}, c_string{"(J)V"});

	java_lang_long_value_field_position =
		java_lang_long_class->instance_field_position(
			c_string{"value_", }, c_string{"J"}
		);

	java_lang_long_class->declared_static_methods().find(
		c_string{"getPrimitiveClass"}, c_string{"()Ljava/lang/Class;"}
	).native_function(
		(void*)+[]() -> object* {
			return & long_class->instance().unsafe_release_without_destroing();
		}
	);
}
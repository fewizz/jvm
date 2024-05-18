#include "decl/lib/java/lang/long.hpp"

#include "decl/classes.hpp"
#include "decl/primitives.hpp"

inline void init_java_lang_long() {
	java_lang_long_class = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/Long"sv
	);

	java_lang_long_constructor =
		java_lang_long_class->declared_instance_methods()
		.find(u8"<init>"sv, u8"(J)V"sv);

	java_lang_long_value_field_position =
		java_lang_long_class->instance_field_position(
			u8"value_"sv, u8"J"sv
		);

	java_lang_long_class->declared_static_methods().find(
		u8"getPrimitiveClass"sv, u8"()Ljava/lang/Class;"sv
	).native_function(
		(void*)+[]() -> object* {
			return long_class->object_ptr();
		}
	);
}
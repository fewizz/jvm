#include "decl/lib/java/lang/class_not_found_exception.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"

static void init_java_lang_class_not_found_exception() {
	class_not_found_exception_class
		= classes.load_class_by_bootstrap_class_loader(
			c_string{ u8"java/lang/ClassNotFoundException" }
		);
}

[[nodiscard]] inline expected<reference, reference>
try_create_class_not_found_exception() {
	c& c = class_not_found_exception_class.get();
	instance_method& m = c.declared_instance_methods().find(
		c_string{ u8"<init>" }, c_string{ u8"()V" }
	);
	return try_create_object(m);
}

[[nodiscard]] inline expected<reference, reference>
try_create_class_not_found_exception(reference message) {
	c& c = class_not_found_exception_class.get();
	instance_method& m = c.declared_instance_methods().find(
		c_string{ u8"<init>" }, c_string{ u8"(Ljava/lang/String;)V" }
	);
	return try_create_object(m, move(message));
}
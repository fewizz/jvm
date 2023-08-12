#include "decl/lib/java/lang/incompatible_class_change_error.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"

#include <optional.hpp>

[[nodiscard]] inline expected<reference, reference>
try_create_incompatible_class_change_error() {
	c& c = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/IncompatibleClassChangeError" }
	);
	instance_method& constructor = c.instance_methods().find(
		c_string{ u8"<init>" }, c_string{ u8"()V" }
	);
	return try_create_object(constructor);
}

[[nodiscard]] inline expected<reference, reference>
try_create_incompatible_class_change_error(j::string& str) {
	c& c = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/IncompatibleClassChangeError" }
	);
	instance_method& constructor = c.instance_methods().find(
		c_string{ u8"<init>" }, c_string{ u8"(Ljava/lang/String;)V" }
	);
	return try_create_object(constructor, str);
}
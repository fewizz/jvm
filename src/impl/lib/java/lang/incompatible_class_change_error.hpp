#include "decl/lib/java/lang/incompatible_class_change_error.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"

#include <optional.hpp>

[[nodiscard]] inline expected<reference, reference>
try_create_incompatible_class_change_error() {
	c& c = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/IncompatibleClassChangeError"s
	);
	instance_method& constructor = c.instance_methods().find(
		u8"<init>"s, u8"()V"s
	);
	return try_create_object(constructor);
}

[[nodiscard]] inline expected<reference, reference>
try_create_incompatible_class_change_error(j::string& str) {
	c& c = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/IncompatibleClassChangeError"s
	);
	instance_method& constructor = c.instance_methods().find(
		u8"<init>"s, u8"(Ljava/lang/String;)V"s
	);
	return try_create_object(constructor, str);
}
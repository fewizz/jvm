#include "decl/lib/java/lang/class_not_found_exception.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"

static void init_java_lang_class_not_found_exception() {
	class_not_found_exception_class
		= classes.load_class_by_bootstrap_class_loader(
			u8"java/lang/ClassNotFoundException"sv
		);
}

[[nodiscard]] inline expected<reference, reference>
try_create_class_not_found_exception() {
	c& c = class_not_found_exception_class.get();
	instance_method& m = c.declared_instance_methods().find(
		u8"<init>"sv, u8"()V"sv
	);
	return try_create_object(m);
}

[[nodiscard]] inline expected<reference, reference>
try_create_class_not_found_exception(reference message) {
	c& c = class_not_found_exception_class.get();
	instance_method& m = c.declared_instance_methods().find(
		u8"<init>"sv, u8"(Ljava/lang/String;)V"sv
	);
	return try_create_object(m, move(message));
}
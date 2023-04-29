#include "decl/lib/java/lang/class_not_found_exception.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"

static void init_java_lang_class_not_found_exception() {
	class_not_found_exception_class
		= classes.load_class_by_bootstrap_class_loader(
			c_string{"java/lang/ClassNotFoundException"}
		);
}

[[nodiscard]] inline expected<reference, reference>
try_create_class_not_found_exception() {
	c& c = class_not_found_exception_class.get();
	method& m = c.declared_instance_methods().find(
		c_string{"<init>"}, c_string{"()V"}
	);
	return try_create_object(m);
}

[[nodiscard]] inline expected<reference, reference>
try_create_class_not_found_exception(reference message) {
	c& c = class_not_found_exception_class.get();
	method& m = c.declared_instance_methods().find(
		c_string{"<init>"}, c_string{"(Ljava/lang/String;)V"}
	);
	return try_create_object(m, move(message));
}
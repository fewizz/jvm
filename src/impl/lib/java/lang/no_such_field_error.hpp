#include "decl/lib/java/lang/no_such_field_error.hpp"

#include "decl/classes.hpp"

static optional<c&> no_such_field_error_class;
static optional<method&> no_such_field_error_constructor;

[[nodiscard]] inline expected<reference, reference>
try_create_no_such_field_error() {
	return try_create_object(
		no_such_field_error_constructor.get()
	);
}

static void init_java_lang_no_such_field_error() {
	no_such_field_error_class = classes.load_class_by_bootstrap_class_loader(
		c_string{"java/lang/NoSuchFieldError"}
	);
	no_such_field_error_constructor
		= no_such_field_error_class->declared_instance_methods().find(
			c_string{"<init>"}, c_string{"()V"}
		);
}
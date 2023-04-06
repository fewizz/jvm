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
	_class& c = class_not_found_exception_class.get();
	method& m = c.declared_instance_methods().find(
		c_string{"<init>"}, c_string{"()V"}
	);

	expected<reference, reference> possible_reference =
		try_create_object(c);
	
	if(possible_reference.is_unexpected()) {
		return unexpected{ move(possible_reference.get_unexpected()) };
	}

	reference ref = move(possible_reference.get_expected());
	optional<reference> possible_throwable = try_execute(m, ref);
	if(possible_throwable.has_value()) {
		return unexpected{ move(possible_throwable.get()) };
	}
	return ref;
}
#include "decl/lib/java/lang/null_pointer_exception.hpp"

#include "decl/object.hpp"
#include "decl/execute.hpp"
#include "decl/classes.hpp"

[[nodiscard]] inline expected<reference, reference>
try_create_null_pointer_exception() {
	method& m = null_pointer_exception_constructor.get();

	return try_create_object(m);
}

inline void init_java_lang_null_pointer_exception() {
	null_pointer_exception_class
		= classes.load_class_by_bootstrap_class_loader(
			c_string{ u8"java/lang/NullPointerException" }
		);

	null_pointer_exception_constructor =
		null_pointer_exception_class
		.get().instance_methods().find(
			c_string{ u8"<init>" }, c_string{ u8"()V" }
		);
}
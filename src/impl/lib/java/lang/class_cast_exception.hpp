#include "decl/lib/java/lang/class_cast_exception.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"
#include "decl/execute.hpp"

#include <optional.hpp>

static optional<c&> class_cast_exception_class;
static optional<method&> class_cast_exception_constructor;

static void init_java_lang_class_cast_exception() {
	class_cast_exception_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/ClassCastException" }
	);

	class_cast_exception_constructor =
		class_cast_exception_class->declared_instance_methods().find(
			c_string{ u8"<init>" }, c_string{ u8"()V" }
		);
}

[[nodiscard]] inline expected<reference, reference>
try_create_class_cast_exception() {
	return try_create_object(
		class_cast_exception_constructor.get()
	);
}
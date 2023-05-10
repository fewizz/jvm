#include "decl/lib/java/lang/array_store_exception.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"
#include "decl/execute.hpp"

#include <optional.hpp>

static optional<c&> array_store_exception_class;
static optional<method&> array_store_exception_constructor;

static void init_java_lang_array_store_exception() {
	array_store_exception_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/ArrayStoreException" }
	);

	array_store_exception_constructor =
		array_store_exception_class->declared_instance_methods().find(
			c_string{ u8"<init>" }, c_string{ u8"()V" }
		);
}

[[nodiscard]] inline expected<reference, reference>
try_create_array_store_exception() {
	return try_create_object(
		array_store_exception_constructor.get()
	);
}
#include "decl/lib/java/lang/negative_array_size_exception.hpp"

#include "decl/execute.hpp"
#include "decl/classes.hpp"
#include "decl/object.hpp"

[[nodiscard]] inline expected<reference, reference>
try_create_negative_array_size_exception() {
	method& m = negative_array_size_exception_constructor.get();

	return try_create_object(m);
}

inline void init_java_lang_negative_array_size_exception() {
	negative_array_size_exception_class
		= classes.load_class_by_bootstrap_class_loader(
			c_string{ u8"java/lang/NegativeArraySizeException" }
		);

	negative_array_size_exception_constructor =
		negative_array_size_exception_class
		.get().instance_methods().find(
			c_string{ u8"<init>" }, c_string{ u8"()V" }
		);
}
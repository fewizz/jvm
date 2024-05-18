#include "decl/lib/java/lang/negative_array_size_exception.hpp"

#include "decl/execute.hpp"
#include "decl/classes.hpp"
#include "decl/object.hpp"

[[nodiscard]] inline expected<reference, reference>
try_create_negative_array_size_exception() {
	return try_create_object(negative_array_size_exception_constructor.get());
}

inline void init_java_lang_negative_array_size_exception() {
	negative_array_size_exception_class
		= classes.load_class_by_bootstrap_class_loader(
			u8"java/lang/NegativeArraySizeException"sv
		);

	negative_array_size_exception_constructor =
		negative_array_size_exception_class
		.get().instance_methods().find(
			u8"<init>"sv, u8"()V"sv
		);
}
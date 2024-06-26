#include "decl/lib/java/lang/index_out_of_bounds_exception.hpp"

#include "decl/execute.hpp"
#include "decl/object.hpp"
#include "decl/classes.hpp"

[[nodiscard]] inline expected<reference, reference>
try_create_index_of_of_bounds_exception() {
	return try_create_object(
		index_of_of_bounds_exception_constructor.get()
	);
}

inline void init_java_lang_index_of_of_bounds_exception() {
	index_of_of_bounds_exception_class
		= classes.load_class_by_bootstrap_class_loader(
			u8"java/lang/IndexOutOfBoundsException"sv
		);

	index_of_of_bounds_exception_constructor =
		index_of_of_bounds_exception_class.get()
		.instance_methods().find(
			u8"<init>"sv, u8"()V"sv
		);
}
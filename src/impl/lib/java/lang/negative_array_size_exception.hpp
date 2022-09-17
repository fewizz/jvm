#include "decl/lib/java/lang/negative_array_size_exception.hpp"

#include "decl/execute.hpp"
#include "decl/classes.hpp"
#include "decl/object/create.hpp"

static inline reference create_negative_array_size_exception() {
	_class& c = negative_array_size_exception_class.value();
	method& m = negative_array_size_exception_constructor.value();

	reference ref = create_object(c);
	stack_entry se{ ref };
	span<stack_entry, uint16> args{ &se, 1 };

	execute(m, args);

	return move(ref);
}

inline void init_java_lang_negative_array_size_exception() {
	negative_array_size_exception_class = classes.find_or_load(
		c_string{ "java/lang/NegativeArraySizeException" }
	);

	negative_array_size_exception_constructor = *
		negative_array_size_exception_class.value()
		.instance_methods().find(c_string{ "<init>" }, c_string{ "()V" });
}
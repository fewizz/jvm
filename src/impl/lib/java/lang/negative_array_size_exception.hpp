#include "decl/lib/java/lang/negative_array_size_exception.hpp"

#include "decl/execute.hpp"
#include "decl/classes.hpp"
#include "decl/object.hpp"

static inline reference create_negative_array_size_exception() {
	_class& c = negative_array_size_exception_class.get();
	method& m = negative_array_size_exception_constructor.get();

	reference ref = create_object(c);
	stack.emplace_back(ref);
	execute(m);
	return move(ref);
}

inline void init_java_lang_negative_array_size_exception() {
	negative_array_size_exception_class
		= classes.load_class_by_bootstrap_class_loader(
			c_string{ "java/lang/NegativeArraySizeException" }
		);

	negative_array_size_exception_constructor =
		negative_array_size_exception_class
		.get().instance_methods().find(
			c_string{ "<init>" }, c_string{ "()V" }
		);
}
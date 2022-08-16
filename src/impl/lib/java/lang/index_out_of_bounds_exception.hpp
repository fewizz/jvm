#include "decl/lib/java/lang/index_out_of_bounds_exception.hpp"

#include "decl/execute.hpp"
#include "decl/object/create.hpp"
#include "decl/class/load.hpp"

inline reference create_index_of_of_bounds_exception() {
	_class& c = index_of_of_bounds_exception_class.value();
	method& m = index_of_of_bounds_exception_constructor.value();

	reference ref = create_object(c);
	stack_entry se{ ref };
	span<stack_entry, uint16> args{ &se, 1 };

	execute(m, args);

	return move(ref);
}

inline void init_java_lang_index_of_of_bounds_exception() {
	index_of_of_bounds_exception_class =
		load_class(c_string{ "java/lang/IndexOutOfBoundsException" });

	index_of_of_bounds_exception_constructor =
		index_of_of_bounds_exception_class.value()
		.instance_methods().find(
			c_string{ "<init>" }, c_string{ "()V" }
		);
}
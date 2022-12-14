#include "decl/lib/java/lang/index_out_of_bounds_exception.hpp"

#include "decl/execute.hpp"
#include "decl/object.hpp"
#include "decl/classes.hpp"

inline reference create_index_of_of_bounds_exception() {
	_class& c = index_of_of_bounds_exception_class.get();
	method& m = index_of_of_bounds_exception_constructor.get();
	reference o = create_object(c);
	stack.emplace_back(o);
	execute(m);
	return move(o);
}

inline void init_java_lang_index_of_of_bounds_exception() {
	index_of_of_bounds_exception_class = classes.find_or_load(
		c_string{ "java/lang/IndexOutOfBoundsException" }
	);

	index_of_of_bounds_exception_constructor =
		index_of_of_bounds_exception_class.get()
		.instance_methods().find(
			c_string{ "<init>" }, c_string{ "()V" }
		);
}
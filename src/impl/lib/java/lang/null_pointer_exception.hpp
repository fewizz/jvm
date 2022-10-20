#include "decl/lib/java/lang/null_pointer_exception.hpp"

#include "decl/object/create.hpp"
#include "decl/execute.hpp"
#include "decl/classes.hpp"

inline reference create_null_pointer_exception() {
	_class& c = null_pointer_exception_class.value();
	method& m = null_pointer_exception_constructor.value();

	reference ref = create_object(c);
	stack.emplace_back(ref);
	execute(m);

	return move(ref);
}

inline void init_java_lang_null_pointer_exception() {
	null_pointer_exception_class = classes.find_or_load(
		c_string{ "java/lang/NullPointerException" }
	);

	null_pointer_exception_constructor =
		null_pointer_exception_class.value()
		.instance_methods().find(c_string{ "<init>" }, c_string{ "()V" });
}
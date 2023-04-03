#include "decl/lib/java/lang/stack_overflow_error.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"
#include "decl/execute.hpp"

inline _class& get_stack_overflow_error_class() {
	static _class& c = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/StackOverflowError" }
	);
	return c;
}

inline reference create_stack_overflow_error() {
	// dump operating stack
	class stack operating_stack = move(stack);

	// creating new, for object creation
	stack = { 32 };

	_class& c = get_stack_overflow_error_class();
	method& m = c.declared_methods().find(
		c_string{ "<init>" }, c_string{ "()V" }
	);
	reference ref = create_object(c);
	execute(m, ref);

	// return operating stack back
	stack = move(operating_stack);

	return ref;
}
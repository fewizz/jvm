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

[[nodiscard]] inline expected<reference, reference>
try_create_stack_overflow_error() {
	// dump operating stack
	class stack operating_stack = move(stack);

	// creating new, for object creation
	stack = { 32 };
	on_scope_exit return_stack_back = [&] {
		// return operating stack back
		stack = move(operating_stack);
	};

	_class& c = get_stack_overflow_error_class();
	method& m = c.declared_methods().find(
		c_string{ "<init>" }, c_string{ "()V" }
	);
	
	return try_create_object(m);
}
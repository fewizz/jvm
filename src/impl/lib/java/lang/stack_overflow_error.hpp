#include "decl/lib/java/lang/stack_overflow_error.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"
#include "decl/execute.hpp"

inline c& get_stack_overflow_error_class() {
	static c& c = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/StackOverflowError"sv
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

	c& c = get_stack_overflow_error_class();
	instance_method& m = c.declared_instance_methods().find(
		u8"<init>"sv, u8"()V"sv
	);
	
	return try_create_object(m);
}
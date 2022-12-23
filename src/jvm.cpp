#include "decl/classes.hpp"
#include "decl/execution/stack.hpp"
#include "decl/execution/thread.hpp"
#include "impl/impl.hpp"

#include "define/primitive_class.hpp"
#include "executable_path.hpp"

#include <posix/default_error_handler.cpp>

int main (int argc, const char** argv) {
	if(argc != 2) {
		posix::std_err.write_from(c_string{ "usage: 'class name'\n" });
		return 1;
	}

	// TODO replace with somethig more reliable
	executable_path = argv[0];

	init_java_lang_object();

	void_class = define_primitive_class(c_string{ "void"    }, 'V');
	bool_class
		= define_primitive_and_its_array_class(c_string{ "boolean" }, 'Z');
	byte_class
		= define_primitive_and_its_array_class(c_string{ "byte" }, 'B');
	short_class
		= define_primitive_and_its_array_class(c_string{ "short" }, 'S');
	char_class
		= define_primitive_and_its_array_class(c_string{ "char" }, 'C');
	int_class
		= define_primitive_and_its_array_class(c_string{ "int" }, 'I');
	long_class
		= define_primitive_and_its_array_class(c_string{ "long"}, 'J');
	float_class
		= define_primitive_and_its_array_class(c_string{ "float" }, 'F');
	double_class
		= define_primitive_and_its_array_class(c_string{ "double" }, 'D');

	bool_array_class   = bool_class  ->get_array_class();
	byte_array_class   = byte_class  ->get_array_class();
	short_array_class  = short_class ->get_array_class();
	char_array_class   = char_class  ->get_array_class();
	int_array_class    = int_class   ->get_array_class();
	long_array_class   = long_class  ->get_array_class();
	float_array_class  = float_class ->get_array_class();
	double_array_class = double_class->get_array_class();

	init_lib();

	thread = create_thread();

	_class& c = load_class(c_string{ argv[1] }.sized());
	method& m = c.declared_static_methods().try_find(
		c_string{ "main" },
		c_string{ "([Ljava/lang/String;)V" }
	).if_has_no_value([] {
		posix::std_err.write_from(c_string{ "main method is not found\n" });
		abort();
	}).get();

	reference args_array = create_array_of(string_class.get(), 0);
	stack.emplace_back(args_array);
	execute(m);

	if(!thrown.is_null()) {
		reference thrown0 = move(thrown);
		posix::std_err.write_from(c_string{ "unhandled throwable\n" });

		method& print_stack_trace = thrown0->_class().instance_methods().find(
			c_string{ "printStackTrace" }, c_string{ "()V" }
		);

		stack.emplace_back(thrown0);
		execute(print_stack_trace);

		return 1;
	}
}
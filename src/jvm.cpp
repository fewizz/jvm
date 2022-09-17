#include "impl/impl.hpp"

#include "define/primitive_class.hpp"
#include "executable_path.hpp"

int main (int argc, const char** argv) {
	if(argc != 2) {
		posix::std_err().write_from(c_string{ "usage: 'class name'" });
		return 1;
	}

	// TODO replace with somethig more reliable
	executable_path = argv[0];

	init_java_lang_object();

	void_class = define_primitive_class(c_string{ "void"    });
	bool_class =
		define_primitive_and_its_array_classes(c_string{ "boolean" }, 'Z');
	byte_class =
		define_primitive_and_its_array_classes(c_string{ "byte" }, 'B');
	short_class =
		define_primitive_and_its_array_classes(c_string{ "short" }, 'S');
	char_class =
		define_primitive_and_its_array_classes(c_string{ "char" }, 'C');
	int_class =
		define_primitive_and_its_array_classes(c_string{ "int" }, 'I');
	long_class = {
		define_primitive_and_its_array_classes(c_string{ "long"}, 'J')
	};
	float_class =
		define_primitive_and_its_array_classes(c_string{ "float" }, 'F');
	double_class =
		define_primitive_and_its_array_classes(c_string{ "double" }, 'D');

	bool_array_class   = bool_class  ->get_array_class();
	byte_array_class   = byte_class  ->get_array_class();
	short_array_class  = short_class ->get_array_class();
	char_array_class   = char_class  ->get_array_class();
	int_array_class    = int_class   ->get_array_class();
	long_array_class   = long_class  ->get_array_class();
	float_array_class  = float_class ->get_array_class();
	double_array_class = double_class->get_array_class();

	init_lib();

	_class& c = load_class(c_string{ argv[1] }.sized());
	method& m = *c.declared_static_methods().try_find(
		c_string{ "main" },
		c_string{ "([Ljava/lang/String;)V" }
	).if_has_no_value([] {
		posix::std_err().write_from(c_string{ "main method is not found" });
		abort();
	}).value();

	stack_entry args_array = create_array_of(string_class.value(), 0);

	execute(m, arguments_span{ &args_array, 1 });

	if(!thrown.is_null()) {
		posix::std_err().write_from(c_string{ "unhandled throwable" });
		return 1;
	}
}
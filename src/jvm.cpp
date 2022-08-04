#include "impl/impl.hpp"

#include "define/primitive_class.hpp"

#include <inttypes.h>

int main (int argc, const char** argv) {
	if(argc != 2) {
		fputs("usage: 'class name'", stderr);
		return 1;
	}

	// TODO replace with somethig more reliable
	executable_path = argv[0];

	void_class   = define_primitive_class(c_string{ "void"    });
	bool_class = {
		define_primitive_class_and_its_array(c_string{ "boolean" }, 'Z')
	};
	byte_class = {
		define_primitive_class_and_its_array(c_string{ "byte" }, 'B')
	};
	short_class = {
		define_primitive_class_and_its_array(c_string{ "short" }, 'S')
	};
	char_class = {
		define_primitive_class_and_its_array(c_string{ "char" }, 'C')
	};
	int_class = {
		define_primitive_class_and_its_array(c_string{ "int" }, 'I')
	};
	long_class = {
		define_primitive_class_and_its_array(c_string{ "long"}, 'J')
	};
	float_class = {
		define_primitive_class_and_its_array(c_string{ "float" }, 'F')
	};
	double_class = {
		define_primitive_class_and_its_array(c_string{ "double" }, 'D')
	};

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
	method& m = c.find_method(
		c_string{ "main" },
		c_string{ "([Ljava/lang/String;)V" }
	);

	method_with_class mwc{ m, c };

	reference args = create_array_of(string_class.value(), 0);
	stack_entry arg0 = args;

	stack_entry value = execute(
		mwc, arguments_span{ &arg0, 1 }
	);

	if(!thrown.is_null()) {
		fputs("unhandled throwable", stdout);
		return 1;
	}

	if(value.is<jint>()) {
		printf("%" PRId32, (int32) value.get<jint>());
	}
	else if(value.is<jlong>()) {
		printf("%" PRId64, (int64) value.get<jlong>());
	}
	else if(
		value.is<reference>() &&
		&value.get<reference>().object()._class() == &string_class.value()
	) {
		for_each_string_codepoint(
			value.get<reference>().object(),
			[](unicode::code_point cp) {
				fputc(cp, stderr);
			}
		);
		fputc('\n', stderr);
	}
}
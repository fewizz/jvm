#include "class/impl.hpp"
#include "classes/impl.hpp"
#include "execute/impl.hpp"
#include "object/impl.hpp"
#include "native/impl.hpp"
#include "native/jni/environment.hpp"
#include "array.hpp"
#include "executable_path.hpp"
#include "classes/load.hpp"
#include "thrown.hpp"
#include "lib/java_lang/null_pointer_exception.hpp"

#include "lib/init.hpp"

#include <unicode/utf16.hpp>
#include <core/c_string.hpp>
#include <core/equals.hpp>

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

	stack_entry value = invoke(
		mwc, arguments_container{}
	);

	if(!thrown.is_null()) {
		fputs("unhandled throwable", stdout);
		return 1;
	}

	if(value.is<jint>()) {
		printf("%" PRId32, value.get<jint>().value);
	} else
	if(value.is<jlong>()) {
		printf("%" PRId64, value.get<jlong>().value);
	} else
	if(
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
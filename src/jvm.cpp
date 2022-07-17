#include "class/impl.hpp"
#include "classes/impl.hpp"
#include "execute/impl.hpp"
#include "object/impl.hpp"
#include "native/jni/environment.hpp"
#include "array.hpp"
#include "classes/load.hpp"

#include "lib/init.hpp"

#include <unicode/utf16.hpp>
#include <core/c_string.hpp>
#include <core/equals.hpp>

#include <inttypes.h>

int main (int argc, const char** argv) {
	if(argc != 3) {
		fputs("usage: 'class name' 'method name'", stderr);
		return 1;
	}

	void_class   = define_primitive_class(c_string{ "void"    });
	bool_class   = define_primitive_class(c_string{ "boolean" });
	byte_class   = define_primitive_class(c_string{ "byte"    });
	short_class  = define_primitive_class(c_string{ "short"   });
	char_class   = define_primitive_class(c_string{ "char"    });
	int_class    = define_primitive_class(c_string{ "int"     });
	long_class   = define_primitive_class(c_string{ "long"    });
	float_class  = define_primitive_class(c_string{ "float"   });
	double_class = define_primitive_class(c_string{ "double"  });

	bool_array_class   = define_array_class(c_string{ "[Z" });
	byte_array_class   = define_array_class(c_string{ "[B" });
	short_array_class  = define_array_class(c_string{ "[S" });
	char_array_class   = define_array_class(c_string{ "[C" });
	int_array_class    = define_array_class(c_string{ "[I" });
	long_array_class   = define_array_class(c_string{ "[J" });
	float_array_class  = define_array_class(c_string{ "[F" });
	double_array_class = define_array_class(c_string{ "[D" });

	init_lib();

	_class& c = load_class(c_string{ argv[1] }.sized());
	method& m = c.find_method(c_string{ argv[2] }.sized());

	stack_entry fv = execute(method_with_class{ m, c });

	if(fv.is<jint>()) {
		printf("%" PRId32, fv.get<jint>().value);
	} else
	if(fv.is<jlong>()) {
		printf("%" PRId64, fv.get<jlong>().value);
	} else
	if(
		fv.is<reference>() &&
		&fv.get<reference>().object()._class() == &string_class.value()
	) {
		for_each_string_codepoint(
			fv.get<reference>().object(),
			[](unicode::code_point cp) {
				fputc(cp, stderr);
			}
		);
		fputc('\n', stderr);
	}
}
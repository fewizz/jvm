#include "class/impl.hpp"
#include "classes/impl.hpp"
#include "execute/impl.hpp"
#include "object/impl.hpp"
#include "native/impl.hpp"
#include "native/jni/environment.hpp"
#include "array.hpp"
#include "exe_path.hpp"
#include "classes/load.hpp"
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
	exe_path = argv[0];

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
	method& m = c.find_method(
		c_string{ "main" },
		c_string{ "([Ljava/lang/String;)V" }
	);

	method_with_class mwc{ m, c };

	expected<stack_entry, reference> result = execute(
		mwc, args_container{}
	);

	if(result.is_unexpected()) {
		fputs("unexpected", stdout);
		return 1;
	}

	stack_entry value = result.get_expected();

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
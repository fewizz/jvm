#if 0

d=`realpath ${BASH_SOURCE[0]}`
d=`dirname ${d}`
root=`realpath ${d}/../../`

if clang++ \
	-std=c++20 \
	-Wall -Wextra \
	-g3 \
	-static \
	-nostdinc++ \
	-I ${root}/../core/include \
	-I ${root}/../encoding/include \
	-I ${root}/include \
	-o ${root}/build/jvm \
	${d}/jvm.cpp
then
	${root}/build/jvm $@
fi
exit 0
#endif

//#include "native.hpp"

#include "classes/impl.hpp"
#include "class/impl.hpp"
#include "field/impl.hpp"
#include "method/impl.hpp"
#include "object/impl.hpp"
#include "native/jni/environment.hpp"

#include "classes/load.hpp"
#include "execute.hpp"

#include <unicode/utf_16.hpp>

#include <core/c_string.hpp>
#include <core/equals.hpp>

int main (int argc, const char** argv) {
	if(argc != 3) {
		fputs("usage: 'class name' 'method name'", stderr);
		return 1;
	}

	native_functions.emplace_back(
		(void*) (void(*)(jni_environment*)) [](jni_environment*) {

		},
		c_string{ "Java_java_lang_Class_registerNatives" }
	);

	native_functions.emplace_back(
		(void*) (object* (*)(jni_environment*, object*))
		[](jni_environment*, object* name) {
			auto value_location0 = name->_class().try_find_instance_field_index(
				c_string{ "value" }, c_string{ "[B" }
			);
			if(!value_location0.has_value()) {
				fputs("couldn't find 'value' field in 'String'", stderr);
				abort();
			}
			auto value_location = value_location0.value();
			auto& values = name->values()[value_location];
			int64 data {
				values.get<reference>().object().values()[0].get<jlong>()
			};
			int32 data_len {
				values.get<reference>().object().values()[1].get<jint>()
			};
			auto it = (uint8*) data;
			auto end = it + data_len;
			nuint characters_count = 0;
			while(it != end) {
				auto cp = utf_16::decoder<endianness::big>{}(it);
				if(cp.is_unexpected()) {
					abort();
				}
				++characters_count;
			}
			char chars[characters_count];
			it = (uint8*) data;
			characters_count = 0;
			while(it != end) {
				auto cp = utf_16::decoder<endianness::big>{}(it);
				chars[characters_count++] = (uint8) cp.get_expected();
			}
			_class& primitive_class {
				find_or_load_class(span{ chars, characters_count })
			};
			return &primitive_class.object();
		},
		c_string{ "Java_java_lang_Class_getPrimitiveClass" }
	);

	define_primitive_class(c_string{ "void" });
	define_primitive_class(c_string{ "boolean" });
	define_primitive_class(c_string{ "byte" });
	define_primitive_class(c_string{ "short" });
	define_primitive_class(c_string{ "char" });
	define_primitive_class(c_string{ "int" });
	define_primitive_class(c_string{ "long" });
	define_primitive_class(c_string{ "float" });
	define_primitive_class(c_string{ "double" });

	_class& cls = load_class(c_string{ argv[1] }.sized());
	stack_entry fv = execute(cls.find_method(c_string{ argv[2] }));
	if(fv.is<jint>()) {
		printf("%d", fv.get<jint>().value);
	}
	if(
		fv.is<reference>() &&
		equals(
			fv.get<reference>().object()._class().name(),
			c_string{ "java/lang/String" }
		)
	) {
		auto& string_class = fv.get<reference>().object()._class();
		auto value_location0 = string_class.try_find_instance_field_index(
			c_string{ "value" }, c_string{ "[B" }
		);
		if(!value_location0.has_value()) {
			fputs("couldn't find 'value' field in 'String'", stderr); abort();
		}
		auto value_location = value_location0.value();

		auto& values = fv.get<reference>().object().values()[value_location];

		int64 data {
			values.get<reference>().object().values()[0].get<jlong>()
		};
		int32 data_len {
			values.get<reference>().object().values()[1].get<jint>()
		};
		
		auto it = (uint8*) data;
		auto end = it + data_len;
		while(it != end) {
			auto cp = utf_16::decoder<endianness::big>{}(it);
			fputc(cp.get_expected(), stderr);
		}
		fputc('\n', stderr);
	}
}
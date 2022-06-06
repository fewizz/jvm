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

#include "define/instance_fields_to_add.hpp"
#include "field_value.hpp"
#include "field.hpp"
#include "class.hpp"
#include "load.hpp"
#include "execute.hpp"

#include <core/c_string.hpp>
#include <core/equals.hpp>

int main (int argc, const char** argv) {
	if(argc != 3) {
		fputs("usage: 'class name' 'method name'", stderr);
		return 1;
	}

	view_class_file(c_string{ "java/lang/Class" }, [&](FILE* file) {
		fseek(file, 0, SEEK_END);
		nuint size = ftell(file);
		rewind(file);
		uint8* data = (uint8*) malloc(size);
		fread(data, 1, size, file);
		define_class(
			span{ data, size },
			instance_fields_to_add { array {
				field_constructor_args{
					class_file::access_flags{},
					name_index{ 0 },
					descriptor_index{ 0 }
				}
			} }
		);
	});

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
	if(fv.is<int32>()) {
		printf("%d", fv.get<int32>());
	}
	if(
		fv.is<reference>() &&
		equals(
			fv.get<reference>().object()._class().name(),
			c_string{ "java/lang/String" }
		)
	) {
		auto& string_class = fv.get<reference>().object()._class();
		auto value_index0 = string_class.try_find_instance_field_index(
			c_string{ "value" }, c_string{ "[B" }
		);
		if(value_index0.is_unexpected()) {
			fputs("couldn't find 'value' field in 'String'", stderr); abort();
		}
		auto value_index = value_index0.get_expected();

		auto& values = fv.get<reference>().object().values()[value_index];

		auto data {
			values.get<reference>().object().values()[0].get<jlong>().value
		};
		auto data_len {
			values.get<reference>().object().values()[1].get<jint>().value
		};
		
		auto it = (uint8*) data;
		auto end = it + data_len;
		while(it != end) {
			auto cp = utf_16::decoder<endianness::big>{}((uint8*)data);
			fputc(cp.get_expected(), stderr);
		}
		fputc('\n', stderr);
	}
}
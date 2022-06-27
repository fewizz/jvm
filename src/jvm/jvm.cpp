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

#include "class/impl.hpp"
#include "classes/impl.hpp"
#include "execute/impl.hpp"
#include "field/impl.hpp"
#include "method/impl.hpp"
#include "object/impl.hpp"
#include "native/jni/environment.hpp"

#include "classes/load.hpp"

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
		(void*) (jbool(*)(jni_environment*, object*))
		[](jni_environment*, object*) {
			return jbool{ true };
		},
		c_string{ "Java_java_lang_Class_desiredAssertionStatus0" }
	);

	native_functions.emplace_back(
		(void*) (jbool(*)(jni_environment*, object* ths))
		[](jni_environment*, object* ths) {
			auto data_location0 = ths->_class().try_find_instance_field_index(
				c_string{ "classData" }, c_string{ "Ljava/lang/Object;" }
			);
			if(!data_location0.has_value()) {
				fputs("couldn't find classData field in Class class", stderr);
				abort();
			}
			field_index data_location = data_location0.value();
			reference long_ref = ths->values()[data_location].get<reference>();
			int64 value = long_ref.object().values()[0].get<jlong>();
			_class* c = (_class*) value;
			return jbool {
				c == &bool_class.value()  ||
				c == &byte_class.value()  ||
				c == &short_class.value() ||
				c == &char_class.value()  ||
				c == &int_class.value()   ||
				c == &long_class.value()  ||
				c == &float_class.value() ||
				c == &double_class.value()
			};
		},
		c_string{ "Java_java_lang_Class_isPrimitive" }
	);

	native_functions.emplace_back(
		(void*) (object* (*)(jni_environment*, object*))
		[](jni_environment*, ::object* name) {
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
			return &primitive_class.reference().object();
		},
		c_string{ "Java_java_lang_Class_getPrimitiveClass" }
	);

	native_functions.emplace_back(
		(void*) (jbool(*)(jni_environment*)) [](jni_environment*) {
			return jbool{ false };
		},
		c_string{ "Java_jdk_internal_misc_CDS_isDumpingClassList0" }
	);
	native_functions.emplace_back(
		(void*) (jbool(*)(jni_environment*)) [](jni_environment*) {
			return jbool{ false };
		},
		c_string{ "Java_jdk_internal_misc_CDS_isDumpingArchive0" }
	);
	native_functions.emplace_back(
		(void*) (jbool(*)(jni_environment*)) [](jni_environment*) {
			return jbool{ false };
		},
		c_string{ "Java_jdk_internal_misc_CDS_isSharingEnabled0" }
	);
	native_functions.emplace_back(
		(void*) (void(*)(jni_environment*, ::object*))
		[](jni_environment*, ::object*) {

		},
		c_string{ "Java_jdk_internal_misc_CDS_initializeFromArchive" }
	);
	native_functions.emplace_back(
		(void*) (void(*)(jni_environment*))
		[](jni_environment*) {

		},
		c_string{ "Java_jdk_internal_misc_Unsafe_registerNatives" }
	);
	native_functions.emplace_back(
		(void*) (jint(*)(jni_environment*, object*, object*))
		[](jni_environment*, object*, object*) {
			return jint{ 0 };
		},
		c_string{ "Java_jdk_internal_misc_Unsafe_arrayBaseOffset0" }
	);
	native_functions.emplace_back(
		(void*) (jint(*)(jni_environment*, object*, object*))
		[](jni_environment*, object*, object* o) -> jint {
			if(&o->_class() == &bool_array_class.value())   return { 1 };
			if(&o->_class() == &byte_array_class.value())   return { 1 };
			if(&o->_class() == &short_array_class.value())  return { 2 };
			if(&o->_class() == &char_array_class.value())   return { 2 };
			if(&o->_class() == &int_array_class.value())    return { 4 };
			if(&o->_class() == &long_array_class.value())   return { 8 };
			if(&o->_class() == &float_array_class.value())  return { 8 };
			if(&o->_class() == &double_array_class.value()) return { 8 };
			return jint{ 0 };
		},
		c_string{ "Java_jdk_internal_misc_Unsafe_arrayIndexScale0" }
	);

	native_functions.emplace_back(
		(void*) (void(*)(jni_environment*)) [](jni_environment*) {

		},
		c_string{ "Java_java_lang_System_registerNatives" }
	);

	native_functions.emplace_back(
		(void*) (void(*)(jni_environment*, object*, jint, object*, jint, jint))
		[](
			jni_environment*,
			object* src, jint src_pos,
			object* dst, jint dst_pos,
			jint len
		) {
			if(src == nullptr) {
				fputs("src is nullptr", stderr);
				abort();
			}
			if(dst == nullptr) {
				fputs("dst is nullptr", stderr);
				abort();
			}
			nuint element_size = 0;
			if(&dst->_class() == &byte_array_class.value()) {
				element_size = 1;
			}
			else {
				fputs("unknown element type", stderr);
				abort();
			}

			uint8* src_data = (uint8*) (int64) src->values()[0].get<jlong>();
			uint8* dst_data = (uint8*) (int64) dst->values()[0].get<jlong>();
			copy(
				span{ src_data + src_pos * element_size, len * element_size }
			).to(
				span{ dst_data + dst_pos * element_size, len * element_size }
			);
		},
		c_string{ "Java_java_lang_System_arraycopy" }
	);

	void_class   = define_primitive_class(c_string{ "void" });
	bool_class   = define_primitive_class(c_string{ "boolean" });
	byte_class   = define_primitive_class(c_string{ "byte" });
	short_class  = define_primitive_class(c_string{ "short" });
	char_class   = define_primitive_class(c_string{ "char" });
	int_class    = define_primitive_class(c_string{ "int" });
	long_class   = define_primitive_class(c_string{ "long" });
	float_class  = define_primitive_class(c_string{ "float" });
	double_class = define_primitive_class(c_string{ "double" });

	bool_array_class   = define_array_class(c_string{ "[Z" });
	byte_array_class   = define_array_class(c_string{ "[B" });
	short_array_class  = define_array_class(c_string{ "[S" });
	char_array_class   = define_array_class(c_string{ "[C" });
	int_array_class    = define_array_class(c_string{ "[I" });
	long_array_class   = define_array_class(c_string{ "[J" });
	float_array_class  = define_array_class(c_string{ "[F" });
	double_array_class = define_array_class(c_string{ "[D" });

	_class& c = load_class(c_string{ argv[1] }.sized());
	method& m = c.find_method(c_string{ argv[2] });
	stack_entry fv = execute(method_with_class{ m, c });
	if(fv.is<jint>()) {
		printf("%d", fv.get<jint>().value);
	} else
	if(fv.is<jlong>()) {
		printf("%lld", fv.get<jlong>().value);
	} else
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
#pragma once

#include "../../native/functions/container.hpp"
#include "../../array.hpp"
#include "../../classes/find_or_load.hpp"

#include <core/starts_with.hpp>
#include <core/array.hpp>

#include <unicode/utf_16.hpp>

static inline _class& class_from_class_instance(object& class_instance) {
	auto data_location0 {
		class_instance._class().try_find_instance_field_index(
			c_string{ "classData" }, c_string{ "Ljava/lang/Object;" }
		)
	};
	if(!data_location0.has_value()) {
		fputs("couldn't find classData field in Class", stderr);
		abort();
	}
	instance_field_index data_location = data_location0.value();
	reference long_ref {
		class_instance.values()[data_location].get<reference>()
	};
	return *array_data<_class>(long_ref.object());
}

void inline init_java_lang_class() {
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
		(void*) (jbool(*)(jni_environment*, object*))
		[](jni_environment*, object* o) {
			auto class_name = class_from_class_instance(*o).name();
			return jbool {
				starts { class_name }.with(array{ '[' })
			};
		},
		c_string{ "Java_java_lang_Class_isArray" }
	);

	native_functions.emplace_back(
		(void*) (jbool(*)(jni_environment*, object* ths))
		[](jni_environment*, object* o) {
			_class* c_ptr = &class_from_class_instance(*o);
			return jbool {
				c_ptr == bool_class.ptr()  ||
				c_ptr == byte_class.ptr()  ||
				c_ptr == short_class.ptr() ||
				c_ptr == char_class.ptr()  ||
				c_ptr == int_class.ptr()   ||
				c_ptr == long_class.ptr()  ||
				c_ptr == float_class.ptr() ||
				c_ptr == double_class.ptr()
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
			auto& values_ob = values.get<reference>().object();
			uint8* data = array_data<uint8>(values_ob);
			int32 data_len = array_length(values_ob);
			uint8* it = data;
			uint8* end = it + data_len;
			nuint characters_count = 0;
			while(it != end) {
				auto cp = utf_16::decoder<endianness::big>{}(it);
				if(cp.is_unexpected()) {
					abort();
				}
				++characters_count;
			}
			char chars[characters_count];
			it = data;
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
}
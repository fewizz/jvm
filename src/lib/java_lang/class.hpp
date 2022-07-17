#pragma once

#include "../../primitives.hpp"
#include "../../native/functions/container.hpp"
#include "../../array.hpp"
#include "../../object/create.hpp"
#include "../../lib/java_lang/string.hpp"
#include "../../classes/find_or_load.hpp"

#include <core/starts_with.hpp>
#include <core/array.hpp>

#include <unicode/utf16.hpp>
#include <unicode/utf8.hpp>

static optional<_class&> class_class{};
static instance_field_index class_data_index{};
static instance_field_index class_name_index{};

static inline _class& class_from_class_instance(object& class_instance) {
	reference long_ref {
		class_instance.values()[class_data_index].get<reference>()
	};
	return *array_data<_class>(long_ref.object());
}

static inline void init_java_lang_class() {

	class_class = find_or_load_class(c_string{ "java/lang/Class" });
	class_data_index = class_class->find_instance_field_index(
		c_string{ "classData" }, c_string{ "Ljava/lang/Object;" }
	);
	class_name_index = class_class->find_instance_field_index(
		c_string{ "name" }, c_string{ "Ljava/lang/String;" }
	);

	native_functions.emplace_back(
		(void*) (void(*)(jni_environment*)) [](jni_environment*) {
			// chillin
		},
		c_string{ "Java_java_lang_Class_registerNatives" }
	);

	native_functions.emplace_back(
		(void*) (object*(*)(jni_environment*, object*))
		[](jni_environment*, object* ths) -> object* {
			_class& c = class_from_class_instance(*ths);
			auto utf8_name = c.name();

			reference string_ref {
				create_string_from_utf8(utf8_name, string_coder::utf16)
			};

			ths->values()[class_name_index] = string_ref;
			return &string_ref.object();
		},
		c_string{ "Java_java_lang_Class_initClassName" }
	);

	native_functions.emplace_back(
		(void*) (bool(*)(jni_environment*, object*))
		[](jni_environment*, object*) {
			return true;
		},
		c_string{ "Java_java_lang_Class_desiredAssertionStatus0" }
	);

	native_functions.emplace_back(
		(void*) (bool(*)(jni_environment*, object*))
		[](jni_environment*, object* o) {
			auto class_name = class_from_class_instance(*o).name();
			return starts { class_name }.with('[');
		},
		c_string{ "Java_java_lang_Class_isArray" }
	);

	native_functions.emplace_back(
		(void*) (bool(*)(jni_environment*, object* ths))
		[](jni_environment*, object* o) {
			_class* c_ptr = &class_from_class_instance(*o);
			return
				c_ptr == bool_class.ptr()  ||
				c_ptr == byte_class.ptr()  ||
				c_ptr == short_class.ptr() ||
				c_ptr == char_class.ptr()  ||
				c_ptr == int_class.ptr()   ||
				c_ptr == long_class.ptr()  ||
				c_ptr == float_class.ptr() ||
				c_ptr == double_class.ptr();
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
				auto cp = utf16::decoder<endianness::big>{}(it);
				if(cp.is_unexpected()) {
					abort();
				}
				++characters_count;
			}
			char chars[characters_count];
			it = data;
			characters_count = 0;
			while(it != end) {
				auto cp = utf16::decoder<endianness::big>{}(it);
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
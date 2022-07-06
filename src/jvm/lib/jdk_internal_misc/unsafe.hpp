#pragma once

#include "../../native/functions/container.hpp"
#include "../java_lang/string.hpp"
#include "../java_lang/class.hpp"
#include <core/view_on_stack.hpp>

static inline void init_jdk_internal_misc_unsafe() {
	native_functions.emplace_back(
		(void*) (void(*)(jni_environment*))
		[](jni_environment*) {

		},
		c_string{ "Java_jdk_internal_misc_Unsafe_registerNatives" }
	);

	native_functions.emplace_back(
		(void*) (int32(*)(jni_environment*, object*, object*))
		[](jni_environment*, object*, object*) {
			return int32{ 0 };
		},
		c_string{ "Java_jdk_internal_misc_Unsafe_arrayBaseOffset0" }
	);

	native_functions.emplace_back(
		(void*) (int32(*)(jni_environment*, object*, object*))
		[](jni_environment*, object*, object* o) -> int32 {
			_class* c_ptr = &o->_class();
			if(c_ptr == bool_array_class.ptr())   return 1;
			if(c_ptr == byte_array_class.ptr())   return 1;
			if(c_ptr == short_array_class.ptr())  return 2;
			if(c_ptr == char_array_class.ptr())   return 2;
			if(c_ptr == int_array_class.ptr())    return 4;
			if(c_ptr == long_array_class.ptr())   return 8;
			if(c_ptr == float_array_class.ptr())  return 8;
			if(c_ptr == double_array_class.ptr()) return 8;
			return 0;
		},
		c_string{ "Java_jdk_internal_misc_Unsafe_arrayIndexScale0" }
	);

	native_functions.emplace_back(
		(void*) (int64(*)(jni_environment*, object*, object*, object*))
		[](jni_environment*, object*, object* c_instance, object* name) {
			_class& c = class_from_class_instance(*c_instance);
			nuint cps_count = 0;

			for_each_string_codepoint(
				*name, [&](unicode::code_point) { ++cps_count; }
			);

			return view_on_stack<char>{ cps_count }([&](auto cps) {
				nuint char_index = 0;
				for_each_string_codepoint(*name, [&](unicode::code_point cp) {
					cps[char_index++] = cp;
				});
				int64 index = c.find_declared_instance_field_index(cps);
				return index;
			});
		},
		c_string{ "Java_jdk_internal_misc_Unsafe_objectFieldOffset1" }
	);

	native_functions.emplace_back(
		(void*) (void(*)(jni_environment*, object*, object*))
		[](jni_environment*, object*, object* c) {
			class_from_class_instance(*c).initialise_if_need();
		},
		c_string{ "Java_jdk_internal_misc_Unsafe_ensureClassInitialized0" }
	);

	native_functions.emplace_back(
		(void*) (int(*)(jni_environment*, object*, object*, int64))
		[](jni_environment*, object*, object* o, int64 offset) -> int32 {
			auto v_ptr = (volatile int32*) &o->values()[offset].get<jint>();
			return *v_ptr;
		},
		c_string{ "Java_jdk_internal_misc_Unsafe_getIntVolatile" }
	);

	native_functions.emplace_back(
		(void*) (bool(*)(jni_environment*, object*, object*, int64, int32, int32))
		[](
			jni_environment*, object*, object* o,
			int64 offset, int32 expected, int32 x
		) -> bool {
			int32& i = o->values()[offset].get<jint>();
			return __atomic_compare_exchange(
				&i, &expected, &x, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST
			); // TODO
		},
		c_string{ "Java_jdk_internal_misc_Unsafe_compareAndSetInt" }
	);

}
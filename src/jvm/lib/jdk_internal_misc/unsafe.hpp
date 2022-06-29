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
		(void*) (jint(*)(jni_environment*, object*, object*))
		[](jni_environment*, object*, object*) {
			return jint{ 0 };
		},
		c_string{ "Java_jdk_internal_misc_Unsafe_arrayBaseOffset0" }
	);

	native_functions.emplace_back(
		(void*) (jint(*)(jni_environment*, object*, object*))
		[](jni_environment*, object*, object* o) -> jint {
			_class* c_ptr = &o->_class();
			if(c_ptr == bool_array_class.ptr())   return { 1 };
			if(c_ptr == byte_array_class.ptr())   return { 1 };
			if(c_ptr == short_array_class.ptr())  return { 2 };
			if(c_ptr == char_array_class.ptr())   return { 2 };
			if(c_ptr == int_array_class.ptr())    return { 4 };
			if(c_ptr == long_array_class.ptr())   return { 8 };
			if(c_ptr == float_array_class.ptr())  return { 8 };
			if(c_ptr == double_array_class.ptr()) return { 8 };
			return jint{ 0 };
		},
		c_string{ "Java_jdk_internal_misc_Unsafe_arrayIndexScale0" }
	);

	native_functions.emplace_back(
		(void*) (jlong(*)(jni_environment*, object*, object*, object*))
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
				return jlong { index };
			});
		},
		c_string{ "Java_jdk_internal_misc_Unsafe_objectFieldOffset1" }
	);

}
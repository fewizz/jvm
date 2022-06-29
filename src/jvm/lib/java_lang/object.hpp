#pragma once

#include "../../native/functions/container.hpp"
#include "../../class/declaration.hpp"

static inline void init_java_lang_object() {

	native_functions.emplace_back(
		(void*) (jint(*)(jni_environment*, object*))
		[](jni_environment*, object* o) {
			return jint{ (int32) (nuint) o };
		},
		c_string{ "Java_java_lang_Object_hashCode" }
	);

	native_functions.emplace_back(
		(void*) (object*(*)(jni_environment*, object*))
		[](jni_environment*, object* o) {
			return & o->_class().reference().object();
		},
		c_string{ "Java_java_lang_Object_getClass" }
	);

}
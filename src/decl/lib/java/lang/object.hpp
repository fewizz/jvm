#pragma once

#include "native/functions.hpp"
#include "class.hpp"

static inline void init_java_lang_object() {

	native_functions.emplace_back(
		(void*) (int32(*)(jni_environment*, object*))
		[](jni_environment*, object* o) {
			return (int32) (nuint) o;
		},
		c_string{ "Java_java_lang_Object_hashCode" },
		c_string{ "()I" }
	);

	native_functions.emplace_back(
		(void*) (object*(*)(jni_environment*, object*))
		[](jni_environment*, object* o) {
			return & o->_class().instance().object();
		},
		c_string{ "Java_java_lang_Object_getClass" },
		c_string{ "()Ljava/lang/Class;" }
	);

}
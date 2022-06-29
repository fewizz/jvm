#pragma once

#include "../../native/functions/container.hpp"
#include <core/bit_cast.hpp>

static inline void init_java_lang_float() {

	native_functions.emplace_back(
		(void*) (jint(*)(jni_environment*, jfloat))
		[](jni_environment*, jfloat value) {
			return jint{ bit_cast<int32>(value.value) };
		},
		c_string{ "Java_java_lang_Float_floatToRawIntBits" }
	);

}
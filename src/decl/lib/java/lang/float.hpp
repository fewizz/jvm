#pragma once

#include "native/functions.hpp"
#include <core/bit_cast.hpp>

static inline void init_java_lang_float() {

	native_functions.emplace_back(
		(void*) (int32(*)(jni_environment*, float))
		[](jni_environment*, float value) {
			return bit_cast<int32>(value);
		},
		c_string{ "Java_java_lang_Float_floatToRawIntBits" },
		c_string{ "(F)I" }
	);

}
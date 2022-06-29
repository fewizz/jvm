#pragma once

#include "../../native/functions/container.hpp"
#include <core/bit_cast.hpp>
#include <math.h>

static inline void init_java_lang_double() {

	native_functions.emplace_back(
		(void*) (jlong(*)(jni_environment*, jdouble))
		[](jni_environment*, jdouble value) {
			return jlong{ bit_cast<int64>(value.value) };
		},
		c_string{ "Java_java_lang_Double_doubleToRawLongBits" }
	);

	native_functions.emplace_back(
		(void*) (jdouble(*)(jni_environment*, jlong))
		[](jni_environment*, jlong value) {
			double result = bit_cast<double>(value.value);
			if(isnan(result)) {
				result = __builtin_nan("");
			}
			return jdouble{ result };
		},
		c_string{ "Java_java_lang_Double_longBitsToDouble" }
	);

}
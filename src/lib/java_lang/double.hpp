#pragma once

#include "../../native/functions/container.hpp"
#include <core/bit_cast.hpp>
#include <math.h>

static inline void init_java_lang_double() {

	native_functions.emplace_back(
		(void*) (int64(*)(jni_environment*, jdouble))
		[](jni_environment*, jdouble value) {
			return bit_cast<int64>(value.value);
		},
		c_string{ "Java_java_lang_Double_doubleToRawLongBits" }
	);

	native_functions.emplace_back(
		(void*) (double(*)(jni_environment*, int64))
		[](jni_environment*, int64 value) {
			double result = bit_cast<double>(value);
			if(isnan(result)) {
				result = __builtin_nan("");
			}
			return result;
		},
		c_string{ "Java_java_lang_Double_longBitsToDouble" }
	);

}
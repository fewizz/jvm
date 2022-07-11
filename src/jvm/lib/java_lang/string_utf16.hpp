#pragma once

#include "../../native/functions/container.hpp"
#include <core/endianness.hpp>

static inline void init_java_lang_string_utf16() {

	native_functions.emplace_back(
		(void*)( (bool(*)(jni_environment*)) [](jni_environment*) {
			return endianness::native == endianness::big;
		}),
		c_string{ "Java_java_lang_StringUTF16_isBigEndian" }
	);

}
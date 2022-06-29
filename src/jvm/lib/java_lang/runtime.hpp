#pragma once

#include "../../native/functions/container.hpp"

static inline void init_java_lang_runtime() {

	native_functions.emplace_back(
		(void*) (jint(*)(jni_environment*, object*))
		[](jni_environment*, object*) {
			return jint{ 1 }; // TODO
		},
		c_string{ "Java_java_lang_Runtime_availableProcessors" }
	);

}
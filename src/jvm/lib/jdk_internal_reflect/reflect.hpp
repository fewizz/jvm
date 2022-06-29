#pragma once

#include "../../callers.hpp"
#include "../../native/functions/container.hpp"

static inline void init_jdk_internal_reflect() {

	native_functions.emplace_back(
		(void*) (object*(*)(jni_environment*))
		[](jni_environment*) {
			nuint last_index = callers.size() - 1;
			return &callers[last_index - 1].reference().object();
		},
		c_string{ "Java_jdk_internal_reflect_Reflection_getCallerClass" }
	);

}
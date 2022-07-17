#pragma once

#include "../../native/functions/container.hpp"

static inline void init_test_native() {
	native_functions.emplace_back(
		(void*) (int(*)(jni_environment*, float)) [](jni_environment*, float i) {
			return (int) i * 2;
		},
		c_string{ "Java_test_Native_test0" }, c_string{ "()I" }
	);
}
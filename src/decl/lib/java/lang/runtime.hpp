#pragma once

#include "native/functions.hpp"

extern "C" [[ noreturn ]] void exit(int exit_code);

static inline void init_java_lang_runtime() {

	native_functions.emplace_back(
		(void*) (void(*)(jni_environment*, object*, int32))
		[](jni_environment*, object*, int32 status) {
			exit(status);
		},
		c_string{ "Java_java_lang_Runtime_exit" },
		c_string{ "(I)" }
	);

	native_functions.emplace_back(
		(void*) (int32(*)(jni_environment*, object*))
		[](jni_environment*, object*) {
			return 1; // TODO
		},
		c_string{ "Java_java_lang_Runtime_availableProcessors" },
		c_string{ "()I" }
	);

}
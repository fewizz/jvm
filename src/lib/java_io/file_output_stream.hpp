#pragma once

#include "../../native/functions/container.hpp"

static inline void init_java_io_file_output_stream() {

	native_functions.emplace_back(
		(void*) (void(*)(jni_environment*))
		[](jni_environment*) {
			// chillin
		},
		c_string{ "Java_java_io_FileOutputStream_initIDs" }
	);

}
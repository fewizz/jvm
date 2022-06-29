#pragma once

#include "../../native/functions/container.hpp"

static inline void init_java_io_file_descriptor() {

	native_functions.emplace_back(
		(void*) (void(*)(jni_environment*))
		[](jni_environment*) {
			// chillin
		},
		c_string{ "Java_java_io_FileDescriptor_initIDs" }
	);

	native_functions.emplace_back(
		(void*) (int64(*)(jni_environment*, int32))
		[](jni_environment*, int32 /* fd */) {
			// chillin, working thru posix api, no windows handles
			return int64(-1);
		},
		c_string{ "Java_java_io_FileDescriptor_getHandle" }
	);

	native_functions.emplace_back(
		(void*) (bool(*)(jni_environment*, int32))
		[](jni_environment*, int32 /*fd*/ ) {
			return true; // TODO
		},
		c_string{ "Java_java_io_FileDescriptor_getAppend" }
	);

}
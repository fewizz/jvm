#include "decl/native/functions.hpp"

#include <stdio.h>

static void init_java_io_file_descriptor() {
	native_functions.emplace_back(
		(void*) (int64 (*)(jni_environment*)) [](jni_environment*) {
			return (int64) stderr;
		},
		c_string{ "Java_java_io_FileDescriptor_stderr_fd" },
		c_string{ "()J" }
	);

	native_functions.emplace_back(
		(void*) (int64 (*)(jni_environment*)) [](jni_environment*) {
			return (int64) stdin;
		},
		c_string{ "Java_java_io_FileDescriptor_stdin_fd" },
		c_string{ "()J" }
	);

	native_functions.emplace_back(
		(void*) (int64 (*)(jni_environment*)) [](jni_environment*) {
			return (int64) stdout;
		},
		c_string{ "Java_java_io_FileDescriptor_stdout_fd" },
		c_string{ "()J" }
	);
}
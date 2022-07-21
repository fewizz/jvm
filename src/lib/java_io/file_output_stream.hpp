#pragma once

#include "../../native/functions/container.hpp"

#include "array.hpp"
//#include <stdio.h>
#include <unistd.h>

static inline void init_java_io_file_output_stream() {

	native_functions.emplace_back(
		(void*) (int32(*)(jni_environment*, int32, int32))
		[](jni_environment*, int32 fd, int32 value) -> int32 {
			return write(fd, &value, 1);
		},
		c_string{ "Java_java_io_FileOutputStream___write" },
		c_string{ "(II)I" }
	);

	native_functions.emplace_back(
		(void*) (int32(*)(jni_environment*, int32, object*, int32, int32))
		[](jni_environment*, int32 fd, object* a, int32 o, int32 l) -> int32 {
			int8* data = array_data<int8>(*a);
			return write(fd, data + o, l);
		},
		c_string{ "Java_java_io_FileOutputStream___write_b" },
		c_string{ "(I[BII)I" }
	);

	native_functions.emplace_back(
		(void*) (int32(*)(jni_environment*, int32))
		[](jni_environment*, int32 fd) -> int32 {
			return close(fd);
		},
		c_string{ "Java_java_io_FileOutputStream___close" },
		c_string{ "(I)I" }
	);

}
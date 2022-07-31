#pragma once

#include "native/functions.hpp"

#include "array.hpp"
#include <stdio.h>

static inline void init_java_io_file_output_stream() {

	native_functions.emplace_back(
		(void*) (bool (*)(jni_environment*, int64, int32))
		[](jni_environment*, int64 fd, int32 value) {
			return fwrite(&value, 1, 1, (FILE*) fd) == 1;
		},
		c_string{ "Java_java_io_FileOutputStream_write" },
		c_string{ "(JI)Z" }
	);

	native_functions.emplace_back(
		(void*) (bool (*)(jni_environment*, int64, object*, int32, int32))
		[](jni_environment*, int64 fd, object* a, int32 o, int32 l) {
			int8* data = array_data<int8>(*a);
			return fwrite(data + o, 1, l, (FILE*) fd) == nuint{ (uint32) l };
		},
		c_string{ "Java_java_io_FileOutputStream_write_buffer" },
		c_string{ "(J[BII)Z" }
	);

	native_functions.emplace_back(
		(void*) (bool (*)(jni_environment*, int64))
		[](jni_environment*, int64 fd) {
			return fclose((FILE*) fd) == 0;
		},
		c_string{ "Java_java_io_FileOutputStream_close" },
		c_string{ "(J)Z" }
	);

}
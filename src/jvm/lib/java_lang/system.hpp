#pragma once

#include "../../native/functions/container.hpp"
#include "../../classes/find_or_load.hpp"
#include "../../object/create.hpp"
#include "../../array.hpp"

#include <time.h>

static inline void init_pipes() {
	_class& file_output_stream = find_or_load_class(
		c_string{ "java/io/FileOutputStream" }
	);
	file_output_stream.initialise_if_need();

	auto constructor0 = file_output_stream.try_find_method(
		c_string{ "<init>" },
		c_string{ "(Ljava/io/FileDescriptor;)V" }
	);

	if(!constructor0.has_value()) {
		fputs("couldn't find FileOutputStream's constructor", stderr);
		abort();
	}

	method& constructor = constructor0.value();

	_class& file_descriptor = find_or_load_class(
		c_string{ "java/io/FileDescriptor" }
	);
	file_descriptor.initialise_if_need();

	static_field& out = file_descriptor.find_declared_static_field(
		c_string{ "out" },
		c_string{ "java/io/FileDescriptor" }
	);

	reference out_ref = create_object(file_output_stream);

	stack_entry args[]{ out_ref, out.value().get<reference>() };
	execute(
		method_with_class{ constructor, file_output_stream },
		span<stack_entry, uint16>{ args }
	);
}

static inline void init_java_lang_system() {
	native_functions.emplace_back(
		(void*) (void(*)(jni_environment*)) [](jni_environment*) {
			init_pipes();
		},
		c_string{ "Java_java_lang_System_registerNatives" }
	);

	native_functions.emplace_back(
		(void*) (void(*)(jni_environment*, object*, jint, object*, jint, jint))
		[](
			jni_environment*,
			object* src, jint src_pos,
			object* dst, jint dst_pos,
			jint len
		) {
			if(src == nullptr) {
				fputs("src is nullptr", stderr); abort();
			}
			if(dst == nullptr) {
				fputs("dst is nullptr", stderr); abort();
			}
			nuint element_size = 0;
			if(&dst->_class() == &byte_array_class.value()) {
				element_size = 1;
			}
			else {
				fputs("unknown element type", stderr); abort();
			}

			uint8* src_data = array_data<uint8>(*src);
			uint8* dst_data = array_data<uint8>(*dst);
			copy(
				span{ src_data + src_pos * element_size, len * element_size }
			).to(
				span{ dst_data + dst_pos * element_size, len * element_size }
			);
		},
		c_string{ "Java_java_lang_System_arraycopy" }
	);

	native_functions.emplace_back(
		(void*) (jlong(*)(jni_environment*))
		[](jni_environment*) {
			timespec tp;
			int result = clock_gettime(CLOCK_MONOTONIC, &tp);
			if(result != 0) {
				fputs("System.nanoTime(): clock_gettime", stderr);
				abort();
			}
			return jlong{ tp.tv_sec * 1000000000ll + tp.tv_nsec };
		},
		c_string{ "Java_java_lang_System_nanoTime" }
	);
}
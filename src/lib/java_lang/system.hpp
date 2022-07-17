#pragma once

#include "../../native/functions/container.hpp"
#include "../../classes/find_or_load.hpp"
#include "../../object/create.hpp"
#include "../../array.hpp"

#include <time.h>

static optional<_class&> system_class{};

static inline void init_pipes() {
	_class& file_output_stream = find_or_load_class(
		c_string{ "java/io/FileOutputStream" }
	);
	file_output_stream.initialise_if_need();

	auto file_output_stream_constructor = file_output_stream.find_method(
		c_string{ "<init>" },
		c_string{ "(Ljava/io/FileDescriptor;)V" }
	);

	_class& file_descriptor = find_or_load_class(
		c_string{ "java/io/FileDescriptor" }
	);
	file_descriptor.initialise_if_need();

	static_field& out_file_descriptor {
		file_descriptor.find_declared_static_field(
			c_string{ "out" },
			c_string{ "Ljava/io/FileDescriptor;" }
		)
	};

	reference file_output_stream_ref = create_object(file_output_stream);

	stack_entry args0[] {
		file_output_stream_ref, out_file_descriptor.value().get<reference>()
	};
	execute(
		method_with_class{ file_output_stream_constructor, file_output_stream },
		span<stack_entry, uint16>{ args0 }
	);

	_class& print_stream {
		find_or_load_class(c_string{ "java/io/PrintStream" })
	};

	method& print_stream_constructor {
		print_stream.find_method(
			c_string{ "<init>" }, c_string{ "(Ljava/io/OutputStream;Z)V" }
		)
	};

	reference print_stream_ref = create_object(print_stream);

	stack_entry args1[] {
		print_stream_ref, file_output_stream_ref, jint{ 1 }
	};

	execute(
		method_with_class{ print_stream_constructor, print_stream },
		span<stack_entry, uint16>{ args1 }
	);

	static_field& system_out {
		system_class->find_declared_static_field(
			c_string{ "out" },
			c_string{ "Ljava/io/PrintStream;" }
		)
	};

	system_out.value() = print_stream_ref;
}

static inline void init_java_lang_system() {

	system_class = find_or_load_class(c_string{ "java/lang/System" });

	native_functions.emplace_back(
		(void*) (void(*)(jni_environment*)) [](jni_environment*) {
			init_pipes();
		},
		c_string{ "Java_java_lang_System_registerNatives" }
	);

	native_functions.emplace_back(
		(void*) (void(*)(
			jni_environment*, object*, int32, object*, int32, int32
		))
		[](
			jni_environment*,
			object* src, int32 src_pos,
			object* dst, int32 dst_pos,
			int32 len
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
		(void*) (int64(*)(jni_environment*))
		[](jni_environment*) {
			timespec tp;
			int result = clock_gettime(CLOCK_MONOTONIC, &tp);
			if(result != 0) {
				fputs("System.nanoTime(): clock_gettime", stderr);
				abort();
			}
			return int64{ tp.tv_sec * 1000000000ll + tp.tv_nsec };
		},
		c_string{ "Java_java_lang_System_nanoTime" }
	);
}
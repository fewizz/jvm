#pragma once

#include "native/functions/container.hpp"
#include "classes/find_or_load.hpp"
#include "object/create.hpp"
#include "array.hpp"

#include <time.h>

static optional<_class&> system_class{};

static inline void init_java_lang_system() {

	system_class = find_or_load_class(c_string{ "java/lang/System" });

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
		c_string{ "Java_java_lang_System_arraycopy" },
		c_string{ "(Ljava/lang/Object;ILjava/lang/Object;II)V" }
	);

	native_functions.emplace_back(
		(void*) (int64(*)(jni_environment*))
		[](jni_environment*) {
			timespec tp;
			int result = clock_gettime(CLOCK_MONOTONIC, &tp);
			if(result != 0) {
				fputs("clock_gettime", stderr);
				abort();
			}
			return int64{ tp.tv_sec * 1000000000ll + tp.tv_nsec };
		},
		c_string{ "Java_java_lang_System_nanoTime" },
		c_string{ "()J" }
	);
}
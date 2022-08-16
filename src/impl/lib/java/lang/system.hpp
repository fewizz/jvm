#include "decl/lib/java/lang/system.hpp"

#include "decl/native/functions.hpp"
#include "decl/object/create.hpp"
#include "decl/array.hpp"
#include "decl/class/load.hpp"

#include <time.h>

static inline void init_java_lang_system() {

	system_class = load_class(c_string{ "java/lang/System" });

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
			if(&src->_class() != &dst->_class()) {
				fputs("different classes", stderr); abort();
			}
			if(!src->_class().is_array()) {
				fputs("src is not array", stderr); abort();
			}
			if(!dst->_class().is_array()) {
				fputs("dst is not array", stderr); abort();
			}
			
			if(&src->_class() == &byte_array_class.value()) {
				uint8* src_data = array_data<uint8>(*src);
				uint8* dst_data = array_data<uint8>(*dst);
				range {
					span{ src_data + src_pos, (nuint) len }
				}.copy_to(span{ dst_data + dst_pos, (nuint) len });
			}
			else if(!src->_class().is_primitive()) {
				reference* src_data = array_data<reference>(*src);
				reference* dst_data = array_data<reference>(*dst);
				range {
					span{ src_data + src_pos, (nuint) len }
				}.copy_to(span{ dst_data + dst_pos, (nuint) len });
			}
			else {
				fputs("unknown array element type", stderr); abort();
			}
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
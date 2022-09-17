#include "decl/lib/java/lang/system.hpp"

#include "decl/object/create.hpp"
#include "decl/array.hpp"
#include "decl/class/load.hpp"
#include "decl/classes.hpp"
#include "decl/native/interface/environment.hpp"

#include <time.h>

static inline void init_java_lang_system() {

	system_class = classes.find_or_load(c_string{ "java/lang/System" });

	system_class->declared_methods().find(
		c_string{ "arraycopy" },
		c_string{ "(Ljava/lang/Object;ILjava/lang/Object;II)V" }
	)->native_function(
		(void*) (void(*)(
			native_interface_environment*, object*, int32, object*, int32, int32
		))
		[](
			native_interface_environment*,
			object* src, int32 src_pos,
			object* dst, int32 dst_pos,
			int32 len
		) {
			if(src == nullptr) {
				posix::std_err().write_from(c_string{ "src is nullptr" });
				abort();
			}
			if(dst == nullptr) {
				posix::std_err().write_from(c_string{ "dst is nullptr" });
				abort();
			}
			if(&src->_class() != &dst->_class()) {
				abort();
			}
			if(!src->_class().is_array()) {
				abort();
			}
			if(!dst->_class().is_array()) {
				abort();
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
				abort();
			}
		}
	);

	system_class->declared_methods().find(
		c_string{ "nanoTime" }, c_string{ "()J" }
	)->native_function(
		(void*) (int64(*)(native_interface_environment*))
		[](native_interface_environment*) {
			timespec tp;
			int result = clock_gettime(CLOCK_MONOTONIC, &tp);
			if(result != 0) {
				abort();
			}
			return int64{ tp.tv_sec * 1000000000ll + tp.tv_nsec };
		}
	);
}
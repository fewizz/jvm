#pragma once

#include "native/function/s/container.hpp"
#include "array.hpp"
#include "thrown.hpp"
#include "lib/java/lang/negative_array_size_exception.hpp"
#include "lib/java/lang/class.hpp"

static void init_java_lang_reflect_array() {

	native_functions.emplace_back(
		(void*) (object*(*)(jni_environment*, object*, int32))
		[](jni_environment*, object* component_type, int32 len) -> object* {
			if(len < 0) {
				thrown = create_negative_array_size_exception();
				return nullptr;
			}
			_class& c = class_from_class_instance(*component_type);
			reference array = create_array_of(c, len);
			return & array.unsafe_release_without_destroing();
		},
		c_string{ "Java_java_lang_reflect_Array_newInstance" },
		c_string{ "(Ljava/lang/Class;I)Ljava/lang/Object;" }
	);

}
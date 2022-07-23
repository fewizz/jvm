#pragma once

#include "java/io/file_descriptor.hpp"
#include "java/io/file_output_stream.hpp"
#include "java/lang/reflect/array.hpp"
#include "java/lang/class.hpp"
#include "java/lang/double.hpp"
#include "java/lang/float.hpp"
#include "java/lang/integer.hpp"
#include "java/lang/negative_array_size_exception.hpp"
#include "java/lang/null_pointer_exception.hpp"
#include "java/lang/object.hpp"
#include "java/lang/runtime.hpp"
#include "java/lang/stack_trace_element.hpp"
#include "java/lang/string.hpp"
#include "java/lang/system.hpp"
#include "java/lang/throwable.hpp"
// #include "jdk_internal_misc/cds.hpp"
// #include "jdk_internal_misc/unsafe.hpp"
// #include "jdk_internal_reflect/reflect.hpp"*/

static inline void init_lib() {
	init_java_io_file_descriptor();
	init_java_io_file_output_stream();
	init_java_lang_reflect_array();
	init_java_lang_class();
	init_java_lang_double();
	init_java_lang_float();
	init_java_lang_integer();
	init_java_lang_negative_array_size_exception();
	init_java_lang_null_pointer_exception();
	init_java_lang_object();
	init_java_lang_runtime();
	init_java_lang_stack_trace_element();
	init_java_lang_string();
	init_java_lang_system();
	init_java_lang_throwable();
	// init_jdk_internal_misc_cds();
	// init_jdk_internal_misc_unsafe();
	// init_jdk_internal_reflect();
}
#pragma once

#include "java_io/file_descriptor.hpp"
#include "java_io/file_output_stream.hpp"
#include "java_lang/class.hpp"
// #include "java_lang/double.hpp"
// #include "java_lang/float.hpp"
#include "java_lang/null_pointer_exception.hpp"
#include "java_lang/object.hpp"
#include "java_lang/runtime.hpp"
#include "java_lang/stack_trace_element.hpp"
#include "java_lang/string.hpp"
#include "java_lang/system.hpp"
#include "java_lang/throwable.hpp"
// #include "jdk_internal_misc/cds.hpp"
// #include "jdk_internal_misc/unsafe.hpp"
// #include "jdk_internal_reflect/reflect.hpp"*/

static inline void init_lib() {
	init_java_io_file_descriptor();
	init_java_io_file_output_stream();
	init_java_lang_class();
	// init_java_lang_double();
	// init_java_lang_float();
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
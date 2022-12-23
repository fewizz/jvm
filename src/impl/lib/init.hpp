#pragma once

#include "./java/io/file_descriptor.hpp"
#include "./java/io/file_output_stream.hpp"
#include "./java/io/serializable.hpp"
#include "./java/lang/invoke/lambda_meta_factory.hpp"
#include "./java/lang/invoke/method_handle.hpp"
#include "./java/lang/invoke/method_handles_lookup.hpp"
#include "./java/lang/invoke/method_type.hpp"
#include "./java/lang/reflect/array.hpp"
#include "./java/lang/array_store_exception.hpp"
#include "./java/lang/class_cast_exception.hpp"
#include "./java/lang/class.hpp"
#include "./java/lang/cloneable.hpp"
#include "./java/lang/double.hpp"
#include "./java/lang/float.hpp"
#include "./java/lang/index_out_of_bounds_exception.hpp"
#include "./java/lang/integer.hpp"
#include "./java/lang/negative_array_size_exception.hpp"
#include "./java/lang/null_pointer_exception.hpp"
#include "./java/lang/object.hpp"
#include "./java/lang/stack_overflow_error.hpp"
#include "./java/lang/runtime.hpp"
#include "./java/lang/stack_trace_element.hpp"
#include "./java/lang/string.hpp"
#include "./java/lang/system.hpp"
#include "./java/lang/thread.hpp"
#include "./java/lang/throwable.hpp"
#include "./java/lang/void.hpp"
#include "./java/nio/buffer.hpp"
// #include "jdk_internal_misc/cds.hpp"
// #include "jdk_internal_misc/unsafe.hpp"
// #include "jdk_internal_reflect/reflect.hpp"*/

static inline void init_lib() {
	init_java_io_file_descriptor();
	init_java_io_file_output_stream();
	init_java_io_serializable();
	init_java_lang_invoke_lambda_meta_factory();
	init_java_lang_invoke_method_handle();
	init_java_lang_invoke_method_handles_lookup();
	init_java_lang_invoke_method_type();
	init_java_lang_reflect_array();
	init_java_lang_array_store_exception();
	init_java_lang_class_cast_exception();
	init_java_lang_class();
	init_java_lang_class();
	init_java_lang_cloneable();
	init_java_lang_double();
	init_java_lang_float();
	init_java_lang_index_of_of_bounds_exception();
	init_java_lang_integer();
	init_java_lang_negative_array_size_exception();
	init_java_lang_null_pointer_exception();
	init_java_lang_runtime();
	init_java_lang_stack_trace_element();
	init_java_lang_string();
	init_java_lang_system();
	init_java_lang_thread();
	init_java_lang_throwable();
	init_java_lang_void();
	init_java_nio_buffer();
	// init_jdk_internal_misc_cds();
	// init_jdk_internal_misc_unsafe();
	// init_jdk_internal_reflect();
}
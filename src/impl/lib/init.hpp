#pragma once

#include "./java/io/file_descriptor.hpp"
#include "./java/io/file_output_stream.hpp"
#include "./java/io/serializable.hpp"
#include "./java/lang/invoke/lambda_meta_factory.hpp"
#include "./java/lang/invoke/method_handle.hpp"
#include "./java/lang/invoke/method_handles.hpp"
#include "./java/lang/invoke/method_handles_lookup.hpp"
#include "./java/lang/invoke/method_type.hpp"
#include "./java/lang/invoke/var_handle.hpp"
#include "./java/lang/invoke/wrong_method_type_exception.hpp"
#include "./java/lang/reflect/array.hpp"
#include "./java/lang/abstract_method_error.hpp"
#include "./java/lang/array_store_exception.hpp"
#include "./java/lang/boolean.hpp"
#include "./java/lang/byte.hpp"
#include "./java/lang/character.hpp"
#include "./java/lang/class_cast_exception.hpp"
#include "./java/lang/class_loader.hpp"
#include "./java/lang/class_not_found_exception.hpp"
#include "./java/lang/class.hpp"
#include "./java/lang/cloneable.hpp"
#include "./java/lang/double.hpp"
#include "./java/lang/float.hpp"
#include "./java/lang/illegal_access_error.hpp"
#include "./java/lang/incompatible_class_change_error.hpp"
#include "./java/lang/index_out_of_bounds_exception.hpp"
#include "./java/lang/integer.hpp"
#include "./java/lang/long.hpp"
#include "./java/lang/negative_array_size_exception.hpp"
#include "./java/lang/no_such_field_error.hpp"
#include "./java/lang/no_such_method_error.hpp"
#include "./java/lang/null_pointer_exception.hpp"
#include "./java/lang/object.hpp"
#include "./java/lang/stack_overflow_error.hpp"
#include "./java/lang/runtime.hpp"
#include "./java/lang/short.hpp"
#include "./java/lang/stack_trace_element.hpp"
#include "./java/lang/string.hpp"
#include "./java/lang/system.hpp"
#include "./java/lang/thread.hpp"
#include "./java/lang/throwable.hpp"
#include "./java/lang/void.hpp"
#include "./java/nio/buffer.hpp"

#include "./jvm/mh/class_member.hpp"
#include "./jvm/mh/constructor.hpp"
#include "./jvm/mh/getter.hpp"
#include "./jvm/mh/invoke_adapter.hpp"
#include "./jvm/mh/setter.hpp"
#include "./jvm/mh/special.hpp"
#include "./jvm/mh/static_getter.hpp"
#include "./jvm/mh/static_setter.hpp"
#include "./jvm/mh/static.hpp"
#include "./jvm/mh/virtual.hpp"
#include "./jvm/app_class_loader.hpp"

static inline void init_lib() {
	init_java_io_file_descriptor();
	init_java_io_file_output_stream();
	init_java_io_serializable();
	init_java_lang_invoke_lambda_meta_factory();
	init_java_lang_invoke_method_handle();
	init_java_lang_invoke_method_handles();
	init_java_lang_invoke_method_handles_lookup();
	init_java_lang_invoke_method_type();
	init_java_lang_invoke_var_handle();
	init_java_lang_invoke_wrong_method_type_exception();
	init_java_lang_reflect_array();
	init_java_lang_array_store_exception();
	init_java_lang_boolean();
	init_java_lang_byte();
	init_java_lang_character();
	init_java_lang_class_cast_exception();
	init_java_lang_class_loader();
	init_java_lang_class_not_found_exception();
	init_java_lang_class();
	init_java_lang_cloneable();
	init_java_lang_double();
	init_java_lang_float();
	init_java_lang_index_of_of_bounds_exception();
	init_java_lang_integer();
	init_java_lang_long();
	init_java_lang_negative_array_size_exception();
	init_java_lang_no_such_field_error();
	init_java_lang_no_such_method_error();
	init_java_lang_null_pointer_exception();
	init_java_lang_runtime();
	init_java_lang_short();
	init_java_lang_stack_trace_element();
	init_java_lang_string();
	init_java_lang_system();
	init_java_lang_thread();
	init_java_lang_throwable();
	init_java_lang_void();
	init_java_nio_buffer();

	init_jvm_mh_class_member();
	init_jvm_mh_constructor();
	init_jvm_mh_getter();
	init_jvm_mh_invoke_adapter();
	init_jvm_mh_setter();
	init_jvm_mh_special();
	init_jvm_mh_static_getter();
	init_jvm_mh_static_setter();
	init_jvm_mh_static();
	init_jvm_mh_virtual();
	init_jvm_app_class_loader();
}
#pragma once

#include "./jni_call.hpp"

#include "primitives.hpp"
#include "object/decl.hpp"

struct jni_environment;
struct jmethod_id;

struct jni_native_interface {
	void* reserved_0{ nullptr };
	void* reserved_1{ nullptr };
	void* reserved_2{ nullptr };
	void* reserved_3{ nullptr };

	jint (JNI_CALL* get_version)(jni_environment* env);
	object* (JNI_CALL* define_class)(
		jni_environment* env, const char *name, object* loader,
		const jbyte *buf, jint len
	);
	object* (JNI_CALL* find_class)(jni_environment* env, const char *name);

	jmethod_id* (JNI_CALL* from_reflected_method)(
		jni_environment* env, object* method
	);
	jmethod_id* (JNI_CALL* from_reflected_field)(
		jni_environment* env, object* field
	);
};
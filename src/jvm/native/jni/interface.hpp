#pragma once

#include "jni_call.hpp"
#include "../../field_value.hpp"

struct jni_env;

struct jni_native_interface {
	void* reserved_0{ nullptr };
	void* reserved_1{ nullptr };
	void* reserved_2{ nullptr };
	void* reserved_3{ nullptr };

	jint (JNI_CALL* GetVersion)(jni_env* env);
	object* (JNI_CALL* DefineClass)(
		jni_env* env, const char *name, object* loader,
		const jbyte *buf, jint len
	);
	object* (JNI_CALL* FindClass)(jni_env* env, const char *name);

	
};
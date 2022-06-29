#pragma once

#include "../../native/functions/container.hpp"

static inline void init_jdk_internal_misc_cds() {

	native_functions.emplace_back(
		(void*) (jbool(*)(jni_environment*)) [](jni_environment*) {
			return jbool{ false };
		},
		c_string{ "Java_jdk_internal_misc_CDS_isDumpingClassList0" }
	);

	native_functions.emplace_back(
		(void*) (jbool(*)(jni_environment*)) [](jni_environment*) {
			return jbool{ false };
		},
		c_string{ "Java_jdk_internal_misc_CDS_isDumpingArchive0" }
	);

	native_functions.emplace_back(
		(void*) (jbool(*)(jni_environment*)) [](jni_environment*) {
			return jbool{ false };
		},
		c_string{ "Java_jdk_internal_misc_CDS_isSharingEnabled0" }
	);

	native_functions.emplace_back(
		(void*) (void(*)(jni_environment*, ::object*))
		[](jni_environment*, ::object*) {

		},
		c_string{ "Java_jdk_internal_misc_CDS_initializeFromArchive" }
	);

	native_functions.emplace_back(
		(void*) (jlong(*)(jni_environment*))
		[](jni_environment*) {
			return jlong{ 0 };
		},
		c_string{ "Java_jdk_internal_misc_CDS_getRandomSeedForDumping" }
	);
}
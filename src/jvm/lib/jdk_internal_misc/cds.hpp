#pragma once

#include "../../native/functions/container.hpp"

static inline void init_jdk_internal_misc_cds() {

	native_functions.emplace_back(
		(void*) (bool(*)(jni_environment*)) [](jni_environment*) {
			return false;
		},
		c_string{ "Java_jdk_internal_misc_CDS_isDumpingClassList0" }
	);

	native_functions.emplace_back(
		(void*) (bool(*)(jni_environment*)) [](jni_environment*) {
			return false;
		},
		c_string{ "Java_jdk_internal_misc_CDS_isDumpingArchive0" }
	);

	native_functions.emplace_back(
		(void*) (bool(*)(jni_environment*)) [](jni_environment*) {
			return false;
		},
		c_string{ "Java_jdk_internal_misc_CDS_isSharingEnabled0" }
	);

	native_functions.emplace_back(
		(void*) (void(*)(jni_environment*, ::object*))
		[](jni_environment*, ::object*) {
			// chillin
		},
		c_string{ "Java_jdk_internal_misc_CDS_initializeFromArchive" }
	);

	native_functions.emplace_back(
		(void*) (int64(*)(jni_environment*))
		[](jni_environment*) {
			return int64{ 0 };
		},
		c_string{ "Java_jdk_internal_misc_CDS_getRandomSeedForDumping" }
	);
}
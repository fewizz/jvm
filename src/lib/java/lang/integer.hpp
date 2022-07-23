#pragma once

#include "class/decl.hpp"
#include "native/functions/container.hpp"
#include "primitives.hpp"

static void init_java_lang_integer() {

	native_functions.emplace_back(
		(void*) (object*(*)()) []() -> object* {
			return int_class->reference().object_ptr();
		},
		c_string{ "Java_java_lang_Integer_getPrimitiveClass" },
		c_string{ "()Ljava/lang/Class;" }
	);

}
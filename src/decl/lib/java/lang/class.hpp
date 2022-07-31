#pragma once

#include "primitives.hpp"
#include "native/functions.hpp"
#include "array.hpp"
#include "object/create.hpp"
#include "lib/java/lang/string.hpp"
#include "class/load.hpp"

#include <core/starts_with.hpp>
#include <core/array.hpp>

#include <unicode/utf16.hpp>
#include <unicode/utf8.hpp>

static optional<_class&> class_class{};
static instance_field_index class_ptr_field_index{};
//static instance_field_index class_name_field_index{};

static inline _class& class_from_class_instance(object& class_instance) {
	return * (_class*) (int64)
		class_instance.values()[class_ptr_field_index].get<jlong>();
}

static inline void init_java_lang_class() {
	class_class = load_class(c_string{ "java/lang/Class" });
	class_ptr_field_index = class_class->find_instance_field_index(
		c_string{ "ptr_" }, c_string{ "J" }
	);

	native_functions.emplace_back(
		(void*) (object*(*)(jni_environment*, object*))
		[](jni_environment*, object* ths) -> object* {
			_class& c = class_from_class_instance(*ths);
			return & c.get_component_class().instance().object();
		},
		c_string{ "Java_java_lang_Class_getComponentType" },
		c_string{ "()Ljava/lang/Class;" }
	);
}
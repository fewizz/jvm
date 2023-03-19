#pragma once

#include "classes.hpp"
#include "decl/primitives.hpp"

inline void init_java_lang_long() {
	_class& c = classes.find_or_load(c_string{"java/lang/Long"});

	c.declared_static_methods().find(
		c_string{"getPrimitiveClass"}, c_string{"()Ljava/lang/Class;"}
	).native_function(
		(void*)+[]() -> object* {
			return & long_class->instance().unsafe_release_without_destroing();
		}
	);
}
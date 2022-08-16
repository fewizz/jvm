#include "decl/lib/java/lang/object.hpp"

#include "decl/native/functions.hpp"
#include "decl/class.hpp"
#include "decl/class/load.hpp"

static inline void init_java_lang_object() {
	object_class = load_class(c_string{ "java/lang/Object" });

	native_functions.emplace_back(
		(void*) (int32(*)(jni_environment*, object*))
		[](jni_environment*, object* o) {
			return (int32) (nuint) o;
		},
		c_string{ "Java_java_lang_Object_hashCode" },
		c_string{ "()I" }
	);

	native_functions.emplace_back(
		(void*) (object*(*)(jni_environment*, object*))
		[](jni_environment*, object* o) {
			return o->_class().instance().object_ptr();
		},
		c_string{ "Java_java_lang_Object_getClass" },
		c_string{ "()Ljava/lang/Class;" }
	);

}
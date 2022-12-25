#include "decl/classes.hpp"
#include "decl/native/environment.hpp"

#include <bit_cast.hpp>

static inline void init_java_lang_float() {
	_class& float_class = classes.find_or_load(c_string{ "java/lang/Float" });
	float_class.declared_methods().find(
		c_string{ "floatToRawIntBits" }, c_string{ "(F)I" }
	).native_function(
		(void*)+[](native_environment*, float value) {
			return bit_cast<int32>(value);
		}
	);

	float_class.declared_methods().find(
		c_string{ "isNaN" }, c_string{ "(F)Z" }
	).native_function(
		(void*)+[](native_environment*, float value) {
			return __builtin_isnan(value);
		}
	);
}
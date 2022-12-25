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
		c_string{ "floatToIntBits" }, c_string{ "(F)I" }
	).native_function(
		(void*)+[](native_environment*, float value) -> int32 {
			if(__builtin_isinf(value)) {
				if(value > 0.0F) {
					return 0x7f800000;
				}
				else {
					return 0xff800000;
				}
			}
			if(__builtin_isnan(value)) {
				return 0x7fc00000;
			}
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
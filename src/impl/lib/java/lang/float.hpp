#include "decl/classes.hpp"
#include "decl/native/environment.hpp"
#include "decl/primitives.hpp"

#include <bit_cast.hpp>

static inline void init_java_lang_float() {
	_class& c = classes.find_or_load(c_string{ "java/lang/Float" });

	c.declared_static_methods().find(
		c_string{"getPrimitiveClass"}, c_string{"()Ljava/lang/Class;"}
	).native_function(
		(void*)+[]() -> object* {
			return & float_class->instance().unsafe_release_without_destroing();
		}
	);

	c.declared_methods().find(
		c_string{ "floatToRawIntBits" }, c_string{ "(F)I" }
	).native_function(
		(void*)+[](native_environment*, float value) {
			return bit_cast<int32>(value);
		}
	);

	c.declared_methods().find(
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

	c.declared_methods().find(
		c_string{ "isNaN" }, c_string{ "(F)Z" }
	).native_function(
		(void*)+[](native_environment*, float value) {
			return __builtin_isnan(value);
		}
	);
}
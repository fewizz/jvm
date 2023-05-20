#include "decl/lib/java/lang/float.hpp"

#include "decl/classes.hpp"
#include "decl/native/environment.hpp"
#include "decl/primitives.hpp"

#include <bit_cast.hpp>

static inline void init_java_lang_float() {
	java_lang_float_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/Float" }
	);

	java_lang_float_constructor =
		java_lang_float_class->declared_instance_methods()
		.find(c_string{ u8"<init>" }, c_string{ u8"(F)V" });

	java_lang_float_value_field_position =
		java_lang_float_class->instance_field_position(
			c_string{ u8"value_" }, c_string{ u8"F" }
		);

	java_lang_float_class->declared_static_methods().find(
		c_string{ u8"getPrimitiveClass" }, c_string{ u8"()Ljava/lang/Class;" }
	).native_function(
		(void*)+[]() -> object* {
			return float_class->object_ptr();
		}
	);

	java_lang_float_class->declared_static_methods().find(
		c_string{ u8"floatToRawIntBits" }, c_string{ u8"(F)I" }
	).native_function(
		(void*)+[](native_environment*, float value) {
			return bit_cast<int32>(value);
		}
	);

	java_lang_float_class->declared_static_methods().find(
		c_string{ u8"floatToIntBits" }, c_string{ u8"(F)I" }
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

	java_lang_float_class->declared_static_methods().find(
		c_string{ u8"isNaN" }, c_string{ u8"(F)Z" }
	).native_function(
		(void*)+[](native_environment*, float value) {
			return __builtin_isnan(value);
		}
	);
}
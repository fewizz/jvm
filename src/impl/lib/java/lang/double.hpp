#include "decl/lib/java/lang/double.hpp"

#include "decl/native/environment.hpp"
#include "decl/classes.hpp"

#include <bit_cast.hpp>

#include <posix/math.hpp>

static inline void init_java_lang_double() {
	java_lang_double_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/Double" }
	);

	java_lang_double_constructor =
		java_lang_double_class->declared_instance_methods()
		.find(c_string{ u8"<init>" }, c_string{ u8"(D)V" });

	java_lang_double_value_field_position =
		java_lang_double_class->instance_field_position(
			c_string{ u8"value_" }, c_string{ u8"D" }
		);

	java_lang_double_class->declared_static_methods().find(
		c_string{ u8"getPrimitiveClass" }, c_string{ u8"()Ljava/lang/Class;" }
	).native_function((void*)+[](native_environment*) -> object* {
		return double_class->object_ptr();
	});

	java_lang_double_class->declared_static_methods().find(
		c_string{ u8"doubleToRawLongBits" }, c_string{ u8"(D)J" }
	).native_function(
		(void*)+[](native_environment*, double value) {
			return bit_cast<int64>(value);
		}
	);

	java_lang_double_class->declared_static_methods().find(
		c_string{ u8"longBitsToDouble" }, c_string{ u8"(J)D" }
	).native_function(
		(void*)+[](native_environment*, int64 value) {
			double result = bit_cast<double>(value);
			if(posix::is_nan(result)) {
				result = __builtin_nan("");
			}
			return result;
		}
	);

}
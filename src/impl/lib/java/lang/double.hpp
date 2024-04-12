#include "decl/lib/java/lang/double.hpp"

#include "decl/native/environment.hpp"
#include "decl/classes.hpp"

#include <bit_cast.hpp>

#include <posix/math.hpp>

static inline void init_java_lang_double() {
	java_lang_double_class = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/Double"s
	);

	java_lang_double_constructor =
		java_lang_double_class->declared_instance_methods()
		.find(u8"<init>"s, u8"(D)V"s);

	java_lang_double_value_field_position =
		java_lang_double_class->instance_field_position(
			u8"value_"s, u8"D"s
		);

	java_lang_double_class->declared_static_methods().find(
		u8"getPrimitiveClass"s, u8"()Ljava/lang/Class;"s
	).native_function((void*)+[](native_environment*) -> object* {
		return double_class->object_ptr();
	});

	java_lang_double_class->declared_static_methods().find(
		u8"doubleToRawLongBits"s, u8"(D)J"s
	).native_function(
		(void*)+[](native_environment*, double value) {
			return bit_cast<int64>(value);
		}
	);

	java_lang_double_class->declared_static_methods().find(
		u8"longBitsToDouble"s, u8"(J)D"s
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
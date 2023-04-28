#include "decl/lib/java/lang/double.hpp"

#include "decl/native/environment.hpp"
#include "decl/classes.hpp"

#include <bit_cast.hpp>

#include <posix/math.hpp>

static inline void init_java_lang_double() {
	java_lang_double_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/Double" }
	);

	java_lang_double_constructor =
		java_lang_double_class->declared_instance_methods()
		.find(c_string{"<init>"}, c_string{"(D)V"});

	java_lang_double_value_field_position =
		java_lang_double_class->instance_field_position(
			c_string{"value_", }, c_string{"D"}
		);

	java_lang_double_class->declared_static_methods().find(
		c_string{"getPrimitiveClass"}, c_string{"()Ljava/lang/Class;"}
	).native_function((void*)+[](native_environment*) -> object* {
		return double_class->instance().object_ptr();
	});

	java_lang_double_class->declared_static_methods().find(
		c_string{ "doubleToRawLongBits" }, c_string{ "(D)J" }
	).native_function(
		(void*)+[](native_environment*, double value) {
			return bit_cast<int64>(value);
		}
	);

	java_lang_double_class->declared_static_methods().find(
		c_string{ "longBitsToDouble" }, c_string{ "(J)D" }
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
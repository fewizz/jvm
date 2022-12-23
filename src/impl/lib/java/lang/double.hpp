#include "decl/native/environment.hpp"
#include "decl/classes.hpp"

#include <bit_cast.hpp>

#include <posix/math.hpp>

static inline void init_java_lang_double() {
	_class& double_class = classes.find_or_load(c_string{ "java/lang/Double" });

	double_class.declared_methods().find(
		c_string{ "doubleToRawLongBits" }, c_string{ "(D)J" }
	).native_function(
		(void*)+[](native_environment*, double value) {
			return bit_cast<int64>(value);
		}
	);

	double_class.declared_methods().find(
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
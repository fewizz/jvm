#include "decl/native/interface/environment.hpp"
#include "decl/classes.hpp"

#include <bit_cast.hpp>

#include <math.h>

static inline void init_java_lang_double() {
	_class& double_class = classes.find_or_load(c_string{ "java/lang/Double" });

	double_class.declared_methods().find(
		c_string{ "doubleToRawLongBits" }, c_string{ "(D)J" }
	)->native_function(
		(void*) (int64(*)(native_interface_environment*, jdouble))
		[](native_interface_environment*, jdouble value) {
			return bit_cast<int64>(value);
		}
	);

	double_class.declared_methods().find(
		c_string{ "longBitsToDouble" }, c_string{ "(J)D" }
	)->native_function(
		(void*) (double(*)(native_interface_environment*, int64))
		[](native_interface_environment*, int64 value) {
			double result = bit_cast<double>(value);
			if(isnan(result)) {
				result = __builtin_nan("");
			}
			return result;
		}
	);

}
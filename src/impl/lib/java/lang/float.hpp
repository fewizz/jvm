#include "decl/classes.hpp"
#include "decl/native/interface/environment.hpp"

#include <bit_cast.hpp>

static inline void init_java_lang_float() {
	classes.find_or_load(c_string{ "java/lang/Float" }).declared_methods().find(
		c_string{ "floatToRawIntBits" }, c_string{ "(F)I" }
	)->native_function(
		(void*) (int32(*)(native_interface_environment*, float))
		[](native_interface_environment*, float value) {
			return bit_cast<int32>(value);
		}
	);

}
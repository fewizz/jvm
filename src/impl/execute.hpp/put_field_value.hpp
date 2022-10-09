#include "decl/execution/stack_entry.hpp"
#include "decl/field/value.hpp"
#include "execution/stack.hpp"

#include <integer.hpp>

#include <posix/io.hpp>

inline void put_field_value(field_value& to) {
	to.view([&]<typename ValueType>(ValueType& value) {
		if constexpr(same_as<reference, ValueType>) {
			value = stack.pop_back<reference>();
		}
		else if constexpr(same_as<jint,   ValueType>) {
			value = stack.pop_back<int32>();
		}
		else if constexpr(same_as<jshort, ValueType>) {
			value = (int16) (uint32) stack.pop_back<int32>();
		}
		else if constexpr(same_as<jbyte,  ValueType>) {
			value = (int8) (uint32) stack.pop_back<int32>();
		}
		else if constexpr(same_as<jchar, ValueType>) {
			value = (uint8) (uint32) stack.pop_back<int32>();
		}
		else if constexpr(same_as<jbool, ValueType>) {
			value = jbool{ stack.pop_back<int32>() == 1 };
		}
		else if constexpr(same_as<jfloat, ValueType>) {
			value = stack.pop_back<float>();
		}
		else if constexpr(same_as<jlong, ValueType>) {
			value = stack.pop_back<int64>();
		}
		else if constexpr(same_as<jdouble, ValueType>) {
			value = stack.pop_back<double>();
		}
		else {
			posix::std_err.write_from(
				c_string{ "couldn't put field value\n" }
			);
			abort();
		}
	});
};
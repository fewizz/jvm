#include "execution/stack_entry.hpp"

#include "field/value.hpp"
#include "execution/stack.hpp"

#include <posix/io.hpp>

inline void get_field_value(field_value& value) {
	value.view([]<typename ValueType>(ValueType& value) {
		if constexpr(same_as<reference, ValueType>) {
			stack.emplace_back(value);
		}
		else if constexpr(
			same_as<ValueType, jint>   ||
			same_as<ValueType, jshort> ||
			same_as<ValueType, jchar>  ||
			same_as<ValueType, jbyte>  ||
			same_as<ValueType, jbool>
		) {
			stack.emplace_back((int32) (uint32) value);
		}
		else if constexpr(same_as<jfloat, ValueType>) {
			stack.emplace_back((float) value);
		}
		else if constexpr(same_as<jlong,  ValueType>) {
			stack.emplace_back((int64) value);
		}
		else if constexpr(same_as<jdouble, ValueType>) {
			stack.emplace_back((double) value);
		}
		else {
			posix::std_err.write_from(c_string{ "couldn't get field value\n" });
			abort();
		}
	});
};
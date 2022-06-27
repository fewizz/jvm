#pragma once

#include "stack_entry.hpp"
#include "../field/value.hpp"
#include "../../abort.hpp"
#include <stdio.h>

inline stack_entry get_field_value(
	field_value& value
) {
	return value.view([]<typename ValueType>(ValueType& value) -> stack_entry {
		if constexpr(same_as<reference, ValueType>) {
			return value;
		}
		else if constexpr(
			same_as<ValueType, jint>   ||
			same_as<ValueType, jshort> ||
			same_as<ValueType, jchar>  ||
			same_as<ValueType, jbyte>
		) {
			return jint{ value.value };
		}
		else if constexpr(
			same_as<ValueType, jbool>
		) {
			return jint{ int32(value.value) };
		}
		else if constexpr(
			same_as<jfloat, ValueType> ||
			same_as<jlong,  ValueType> ||
			same_as<jdouble, ValueType>
		) {
			return value;
		}

		fputs("couldn't get field value", stderr); abort();
	});
};
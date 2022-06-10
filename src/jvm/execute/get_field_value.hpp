#pragma once

#include "stack_entry.hpp"
#include "../field_value.hpp"

inline void get_field_value(
	field_value& value, stack_entry* stack, nuint& stack_size
) {
	value.view([&]<typename ValueType>(ValueType& value) {
		if constexpr(same_as<reference, ValueType>) {
			stack[stack_size++] = value;
		} else
		if constexpr(
			same_as<jint,   ValueType> ||
			same_as<jshort, ValueType> ||
			same_as<jchar,  ValueType> ||
			same_as<jbyte,  ValueType>
		) {
			stack[stack_size++] = int32{ value.value };
		} else
		if constexpr(same_as<jfloat, ValueType>) {
			stack[stack_size++] = float{ value.value };
		}
		else {
			fputs("couldn't get field value", stderr); abort();
		}
	});
};
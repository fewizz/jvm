#pragma once

#include "stack_entry.hpp"
#include "../field_value.hpp"

inline void put_field_value(
	field_value& value, stack_entry* stack, nuint& stack_size
) {
	stack_entry stack_value = move(stack[--stack_size]);
	value.view([&]<typename ValueType>(ValueType& value) {
		if constexpr(same_as<reference, ValueType>) {
			value = move(stack_value.get<reference>());
		} else
		if constexpr(
			same_as<jint,   ValueType> ||
			same_as<jshort, ValueType> ||
			same_as<jchar,  ValueType> ||
			same_as<jbyte,  ValueType>
		) {
			value = ValueType {
				(decltype(value.value)) stack_value.get<int32>()
			};
		} else
		if constexpr(same_as<jbool, ValueType>) {
			value = jbool{ stack_value.get<int32>() == 1 };
		} else
		if constexpr(same_as<jfloat, ValueType>) {
			value = jfloat{ stack_value.get<float>() };
		}
		else {
			fputs("couldn't put field value", stderr); abort();
		}
	});
};
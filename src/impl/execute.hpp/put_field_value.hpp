#include "execution/stack_entry.hpp"

#include "field/value.hpp"
#include "abort.hpp"

#include <core/integer.hpp>

#include <stdio.h>

inline void put_field_value(
	field_value& to, stack_entry from
) {
	to.view([&]<typename ValueType>(ValueType& value) {
		if constexpr(same_as<reference, ValueType>) {
			value = move(from.get<reference>());
		}
		else if constexpr(
			same_as<jint,   ValueType> ||
			same_as<jshort, ValueType> ||
			same_as<jbyte,  ValueType>
		) {
			value = ValueType {
				(int_of_bits<sizeof(ValueType) * 8>) from.get<jint>()
			};
		}
		else if constexpr(same_as<jchar, ValueType>) {
			value = jchar{ (uint16) from.get<jint>() };
		}
		else if constexpr(same_as<jbool, ValueType>) {
			value = jbool{ from.get<jint>() == 1 };
		}
		else if constexpr(same_as<jfloat, ValueType>) {
			value = from.get<jfloat>();
		}
		else if constexpr(same_as<jlong, ValueType>) {
			value = from.get<jlong>();
		}
		else if constexpr(same_as<jdouble, ValueType>) {
			value = from.get<jdouble>();
		}
		else {
			fputs("couldn't put field value", stderr);
			abort();
		}
	});
};
#pragma once

#include "object/declaration.hpp"

template<typename Type>
Type* array_data(object& o) {
	return (Type*) (int64) o.values()[0].get<jlong>();
}

template<typename Type>
void array_data(object& o, Type* ptr) {
	o.values()[0].get<jlong>() = (int64) ptr;
}

[[maybe_unused]] static inline int32 array_length(object& o) {
	return (int32) o.values()[1].get<jint>();
}

[[maybe_unused]] static inline void array_length(object& o, int32 length) {
	o.values()[1].get<jint>() = length;
}
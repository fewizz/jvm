#pragma once

#include "object/declaration.hpp"

template<typename Type>
static Type* array_data(object& o) {
	return (Type*) (int64) o.values()[0].get<jlong>();
}

template<typename Type>
static void array_data(object& o, Type* ptr) {
	o.values()[0].get<jlong>() = (int64) ptr;
}

static inline int32 array_length(object& o) {
	return (int32) o.values()[1].get<jint>();
}

static inline void array_length(object& o, int32 length) {
	o.values()[1].get<jint>() = length;
}
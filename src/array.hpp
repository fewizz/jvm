#pragma once

#include "class/decl.hpp"
#include "object/decl.hpp"
#include "object/create.hpp"

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

static inline reference create_object_array_of(_class& type, int32 length) {
	reference* data = (reference*) calloc(length, sizeof(reference));
	_class& array_class = type.find_or_load_array_class();
	reference ref = create_object(array_class);
	array_length(ref.object(), length);
	array_data(ref.object(), data);
	return move(ref);
}
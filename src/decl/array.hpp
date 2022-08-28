#pragma once

#include "class.hpp"
#include "object.hpp"

inline optional<_class&> bool_array_class{};
inline optional<_class&> byte_array_class{};
inline optional<_class&> short_array_class{};
inline optional<_class&> char_array_class{};
inline optional<_class&> int_array_class{};
inline optional<_class&> long_array_class{};
inline optional<_class&> float_array_class{};
inline optional<_class&> double_array_class{};


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

template<typename Type>
static inline span<Type> array_as_span(object& o) {
	return { array_data<Type>(o), (nuint) array_length(o) };
}

template<typename Type>
static inline reference create_array_by_class(
	_class& array_class, int32 length
);

static inline reference create_array_of(
	_class& element_class, int32 length
);

static inline reference create_bool_array(int32 length) {
	return create_array_by_class<jbool>(bool_array_class.value(), length);
}

static inline reference create_byte_array(int32 length) {
	return create_array_by_class<jbyte>(byte_array_class.value(), length);
}

static inline reference create_short_array(int32 length) {
	return create_array_by_class<jshort>(short_array_class.value(), length);
}

static inline reference create_char_array(int32 length) {
	return create_array_by_class<jchar>(char_array_class.value(), length);
}

static inline reference create_int_array(int32 length) {
	return create_array_by_class<jint>(int_array_class.value(), length);
}

static inline reference create_float_array(int32 length) {
	return create_array_by_class<jfloat>(float_array_class.value(), length);
}

static inline reference create_long_array(int32 length) {
	return create_array_by_class<jlong>(long_array_class.value(), length);
}

static inline reference create_double_array(int32 length) {
	return create_array_by_class<jdouble>(double_array_class.value(), length);
}
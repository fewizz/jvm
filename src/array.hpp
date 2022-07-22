#pragma once

#include "class/decl.hpp"
#include "object/decl.hpp"
#include "object/create.hpp"

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


static inline reference create_array_by_class(
	_class& array_class, nuint element_size, int32 length
) {
	reference* data = (reference*) default_allocator{}.allocate_zeroed(
		element_size * sizeof(reference)
	);
	reference ref = create_object(array_class);
	array_length(ref.object(), length);
	array_data(ref.object(), data);
	return move(ref);
}


static inline reference create_array_of(
	_class& element_class, int32 length
) {
	_class& array_class = element_class.get_array_class();
	return create_array_by_class(array_class, sizeof(reference), length);
}

static inline reference create_bool_array(int32 length) {
	return create_array_by_class(
		bool_array_class.value(), sizeof(jbool), length
	);
}

static inline reference create_byte_array(int32 length) {
	return create_array_by_class(
		byte_array_class.value(), sizeof(jbyte), length
	);
}

static inline reference create_short_array(int32 length) {
	return create_array_by_class(
		short_array_class.value(), sizeof(jshort), length
	);
}

static inline reference create_char_array(int32 length) {
	return create_array_by_class(
		char_array_class.value(), sizeof(jchar), length
	);
}

static inline reference create_int_array(int32 length) {
	return create_array_by_class(
		int_array_class.value(), sizeof(jint), length
	);
}

static inline reference create_float_array(int32 length) {
	return create_array_by_class(
		float_array_class.value(), sizeof(jfloat), length
	);
}

static inline reference create_long_array(int32 length) {
	return create_array_by_class(
		long_array_class.value(), sizeof(jlong), length
	);
}

static inline reference create_double_array(int32 length) {
	return create_array_by_class(
		double_array_class.value(), sizeof(jdouble), length
	);
}
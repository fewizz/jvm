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

constexpr inline layout::position
	array_data_field_position{ 0 },
	array_length_field_position{ bytes_in<int64> };

template<typename Type>
static Type* array_data(object& o) {
	return (Type*) o.get<int64>(array_data_field_position);
}

static inline int32 array_length(object& o) {
	return o.get<int32>(array_length_field_position);
}

template<typename Type>
static void array_data(object& o, Type* ptr) {
	o.set(array_data_field_position, (int64) ptr);
}

static inline int32 array_length(object& o);

static inline void array_length(object& o, int32 length) {
	o.set(array_length_field_position, length);
}

template<typename Type>
static inline span<Type> array_as_span(object& o) {
	return { array_data<Type>(o), (nuint) array_length(o) };
}

template<typename Type>
static inline reference create_array_by_class(
	_class& array_class, int32 length
) {
	reference ref = create_object(array_class);
	array_length(ref, length);
	Type* data = (Type*) posix::allocate_raw_zeroed_memory_of<Type>(
		length
	).iterator();
	array_data(ref, data);
	return ref;
}

static inline reference create_array_of(
	_class& element_class, int32 length
) {
	_class& array_class = element_class.get_array_class();
	return create_array_by_class<reference>(array_class, length);
}

static inline reference create_bool_array(int32 length) {
	return create_array_by_class<bool>(bool_array_class.get(), length);
}

static inline reference create_byte_array(int32 length) {
	return create_array_by_class<int8>(byte_array_class.get(), length);
}

static inline reference create_short_array(int32 length) {
	return create_array_by_class<int16>(short_array_class.get(), length);
}

static inline reference create_char_array(int32 length) {
	return create_array_by_class<uint16>(char_array_class.get(), length);
}

static inline reference create_int_array(int32 length) {
	return create_array_by_class<int32>(int_array_class.get(), length);
}

static inline reference create_float_array(int32 length) {
	return create_array_by_class<float>(float_array_class.get(), length);
}

static inline reference create_long_array(int32 length) {
	return create_array_by_class<int64>(long_array_class.get(), length);
}

static inline reference create_double_array(int32 length) {
	return create_array_by_class<double>(double_array_class.get(), length);
}
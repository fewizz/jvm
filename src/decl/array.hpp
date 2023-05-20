#pragma once

#include "class.hpp"
#include "object.hpp"

inline optional<c&> bool_array_class{};
inline optional<c&> byte_array_class{};
inline optional<c&> short_array_class{};
inline optional<c&> char_array_class{};
inline optional<c&> int_array_class{};
inline optional<c&> long_array_class{};
inline optional<c&> float_array_class{};
inline optional<c&> double_array_class{};

constexpr inline layout::position
	array_data_field_position{ 0 },
	array_length_field_position{ bytes_in<int64> };

template<typename Type>
static Type* array_data(o<jl::object>& o) {
	return (Type*) o.get<int64>(array_data_field_position);
}

static inline int32 array_length(o<jl::object>& o) {
	return o.get<int32>(array_length_field_position);
}

static inline int32 array_length(o<jl::object>& o);

static inline void array_length(o<jl::object>& o, int32 length) {
	o.set(array_length_field_position, length);
}

template<typename Type>
static inline span<Type> array_as_span(o<jl::object>& o) {
	return { array_data<Type>(o), (nuint) array_length(o) };
}

template<typename Type>
static inline expected<reference, reference> try_create_array_by_class(
	c& array_class, int32 length
) {
	expected<reference, reference> possible_ref
		= try_create_object(array_class);

	if(possible_ref.is_unexpected()) {
		return unexpected{ possible_ref.move_unexpected() };
	}

	reference ref = possible_ref.move_expected();

	array_length(ref, length);
	span<storage<Type>> data =
		posix::allocate_raw_zeroed<Type>(
			length
		);

	int64 ptr = (int64) data.iterator();

	ref->set(array_data_field_position, ptr);

	return ref;
}

static inline expected<reference, reference> try_create_array_of(
	c& element_class, int32 length
) {
	c& array_class = element_class.get_array_class();
	return try_create_array_by_class<reference>(array_class, length);
}

static inline expected<reference, reference>
try_create_bool_array(int32 length) {
	return try_create_array_by_class<bool>(bool_array_class.get(), length);
}

static inline expected<reference, reference>
try_create_byte_array(int32 length) {
	return try_create_array_by_class<int8>(byte_array_class.get(), length);
}

static inline expected<reference, reference>
try_create_short_array(int32 length) {
	return try_create_array_by_class<int16>(short_array_class.get(), length);
}

static inline expected<reference, reference>
try_create_char_array(int32 length) {
	return try_create_array_by_class<uint16>(char_array_class.get(), length);
}

static inline expected<reference, reference>
try_create_int_array(int32 length) {
	return try_create_array_by_class<int32>(int_array_class.get(), length);
}

static inline expected<reference, reference>
try_create_float_array(int32 length) {
	return try_create_array_by_class<float>(float_array_class.get(), length);
}

static inline expected<reference, reference>
try_create_long_array(int32 length) {
	return try_create_array_by_class<int64>(long_array_class.get(), length);
}

static inline expected<reference, reference>
try_create_double_array(int32 length) {
	return try_create_array_by_class<double>(double_array_class.get(), length);
}
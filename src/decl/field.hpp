#pragma once

#include "./class/member.hpp"
#include "./descriptor.hpp"

#include <class_file/descriptor/type.hpp>

struct field : class_member {
	using class_member::class_member;

	one_of_descriptor_field_types type;
	uint8 stack_size;

	field(
		class_file::access_flags access_flags,
		class_file::constant::utf8 name,
		class_file::constant::utf8 desc
	);
};

struct static_field : field {
};

struct instance_field : field {
	using field::field;
	instance_field(field&& f) : field{ move(f) } {}
};

#include <types.hpp>

template<typename T0, typename T1>
requires (
	type_is_lvalue_reference<T0> &&
	type_is_lvalue_reference<T1> &&
	base_of<remove_reference<T0>, ::field> &&
	base_of<remove_reference<T1>, ::field>
)
struct __types::common::result<T0, T1> {
	using type = field&;
};
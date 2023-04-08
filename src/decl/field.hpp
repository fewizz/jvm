#pragma once

#include "./class/member.hpp"
#include "./descriptor.hpp"

#include <class_file/descriptor/type.hpp>

struct field : class_member {
	using class_member::class_member;

	one_of_descriptor_field_types type;
	uint8 stack_size;

	field(
		class_file::access_flags   access_flags,
		class_file::constant::utf8 name,
		class_file::constant::utf8 desc
	);
};
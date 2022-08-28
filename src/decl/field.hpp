#pragma once

#include "class/member.hpp"

struct field : class_member<class_file::constant::utf8> {
	using base_type = class_member<class_file::constant::utf8>;
	using base_type::base_type;
};
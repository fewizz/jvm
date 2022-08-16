#pragma once

#include <class_file/constant.hpp>

struct class_name : class_file::constant::utf8 {
private:
	using base_type = class_file::constant::utf8;
public:
	using base_type::base_type;

	class_name(utf8 str) : base_type{ str } {}
};

struct this_class_name : class_name {
	using class_name::class_name;
};
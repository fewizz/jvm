#pragma once

#include <class_file/constant.hpp>

struct super_class_index : public class_file::constant::class_index {
	using class_file::constant::class_index::class_index;
};
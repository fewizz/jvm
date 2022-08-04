#pragma once

#include "class/member_with_class.hpp"
#include "class.hpp"

inline class_file::constant::utf8
class_member_with_class::name() {
	return _class().name(class_member());
}

inline class_file::constant::utf8
class_member_with_class::descriptor() {
	return _class().descriptor(class_member());
}
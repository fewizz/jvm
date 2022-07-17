#pragma once

#include "decl.hpp"
#include "../../../class/decl.hpp"

inline class_file::constant::utf8
class_member_with_class::name() {
	return _class().name(class_member());
}

inline class_file::constant::utf8
class_member_with_class::descriptor() {
	return _class().descriptor(class_member());
}
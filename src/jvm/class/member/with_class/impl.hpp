#pragma once

#include "declaration.hpp"
#include "../../../class/declaration.hpp"

class_file::constant::utf8 class_member_with_class::
name() {
	return _class().name(class_member());
}

class_file::constant::utf8 class_member_with_class::
descriptor() {
	return _class().descriptor(class_member());
}
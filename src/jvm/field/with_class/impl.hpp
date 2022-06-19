#pragma once

#include "declaration.hpp"

#include "../../class/impl.hpp"

class_file::constant::utf8 field_with_class::name() const {
	return _class.name(field);
}

class_file::constant::utf8 field_with_class::descriptor() const {
	return _class.descriptor(field);
}
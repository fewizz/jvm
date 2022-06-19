#pragma once

#include "declaration.hpp"

#include "../../class/declaration.hpp"

inline class_file::constant::utf8 method_with_class::descriptor() const {
	return _class.descriptor(method);
}

inline class_file::constant::utf8 method_with_class::name() const {
	return _class.name(method);
}
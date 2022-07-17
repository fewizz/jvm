#pragma once

#include "decl.hpp"

#include "../../class/decl.hpp"

inline class_file::constant::utf8 method_with_class::descriptor() const {
	return _class.descriptor(method);
}

inline class_file::constant::utf8 method_with_class::name() const {
	return _class.name(method);
}
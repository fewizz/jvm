#pragma once

#include "class/file/constant.hpp"

struct _class;
struct method;

struct method_with_class {
	method& method;
	_class& _class;

	inline class_file::constant::utf8 descriptor() const;
	inline class_file::constant::utf8 name() const;
};
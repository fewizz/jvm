#pragma once

#include "class/file/constant.hpp"

struct _class;
struct field;

struct field_with_class {
	field& field;
	_class& _class;

	inline class_file::constant::utf8 name() const;
	inline class_file::constant::utf8 descriptor() const;

};
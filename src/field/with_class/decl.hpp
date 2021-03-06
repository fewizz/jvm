#pragma once

#include "field/decl.hpp"

#include "class/member/with_class/decl.hpp"

#include <class/file/constant.hpp>

struct field_with_class : class_member_with_class {
	using base_type = class_member_with_class;

	field_with_class(field& field, ::_class& _class) :
		base_type{ (::class_member&) field, _class }
	{}

	const ::field& field() const { return (::field&) class_member(); }
	      ::field& field()       { return (::field&) class_member(); }

};
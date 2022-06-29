#pragma once

#include "../declaration.hpp"
#include "../../with_class/declaration.hpp"

struct instance_field_with_class : field_with_class {
	using base_type = field_with_class;

	instance_field_with_class(::instance_field& field, ::_class& _class) :
		base_type{ (::field&) field, _class }
	{}

};
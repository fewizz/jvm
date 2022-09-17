#pragma once

#include "field/value.hpp"

#include <variant.hpp>

struct stack_entry :
	variant<jint, jfloat, jlong, jdouble, reference>
{
	using base_type =
		variant<jint, jfloat, jlong, jdouble, reference>;
	using base_type::base_type;
	using base_type::operator = ;
};
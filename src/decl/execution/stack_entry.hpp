#pragma once

#include "field/value.hpp"

#include <elements/one_of.hpp>

struct stack_entry :
	elements::one_of<jint, jfloat, jlong, jdouble, reference>
{
	using base_type =
		elements::one_of<jint, jfloat, jlong, jdouble, reference>;
	using base_type::base_type;
	using base_type::operator = ;
};
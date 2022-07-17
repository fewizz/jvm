#pragma once

#include "../field/value.hpp"

#include <core/meta/elements/one_of.hpp>

struct stack_entry :
	elements::one_of<jvoid, jint, jfloat, jlong, jdouble, reference>
{
	using base_type =
		elements::one_of<jvoid, jint, jfloat, jlong, jdouble, reference>;
	using base_type::base_type;
	using base_type::operator = ;

	stack_entry() : base_type{ jvoid{} } {}
};
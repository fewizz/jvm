#pragma once

#include "../field/value.hpp"

#include <core/meta/elements/one_of.hpp>

struct stack_entry : elements::one_of<jvoid, int32, float, reference> {
	using base_type = elements::one_of<jvoid, int32, float, reference>;
	using base_type::base_type;

	stack_entry() : base_type{ jvoid{} } {}
};
#pragma once

#include "field.hpp"

#include <core/move.hpp>

struct instance_field : field {
	using base_type = field;
	using base_type::base_type;

	instance_field(field&& f) :
		base_type{ move(f) }
	{}

};
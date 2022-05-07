#pragma once

#include <core/meta/elements/one_of.hpp>

struct object {

};

struct unknown{};

using field_value = elements::one_of<
	unknown, void*, bool, int8, uint16, int16, int32, int64,
	float, double, object&
>;
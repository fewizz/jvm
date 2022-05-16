#pragma once

#include <core/integer.hpp>

struct object;

struct counted_object_ptr {
	nuint count;
	object* object_ptr;
};
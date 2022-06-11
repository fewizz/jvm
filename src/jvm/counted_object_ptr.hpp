#pragma once

#include <core/meta/elements/optional.hpp>

struct object;

struct counted_object_ptr {
	nuint count;
	optional<object&> object;
};
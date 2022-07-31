#pragma once

#include <core/optional.hpp>
#include "object/reference.hpp"

struct _class;
struct method;

struct execution_context {
	_class& _class;
	method& method;
	optional<execution_context&> previous;
};
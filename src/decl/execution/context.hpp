#pragma once

#include "object/reference.hpp"

#include <optional.hpp>

struct _class;
struct method;

struct execution_context {
	_class& _class;
	method& method;
	optional<execution_context&> previous;
};
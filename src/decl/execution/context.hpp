#pragma once

#include "reference.hpp"
#include "./stack.hpp"

#include <optional.hpp>

struct _class;
struct method;

struct execution_context {
	method& method;
	optional<execution_context&> previous;
};
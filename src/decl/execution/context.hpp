#pragma once

#include "object/reference.hpp"
#include "./stack.hpp"

#include <optional.hpp>

struct _class;
struct method;

struct execution_context {
	method& method;
	//span<stack_entry> frame;
	optional<execution_context&> previous;
};
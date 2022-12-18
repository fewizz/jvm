#pragma once

#include "reference.hpp"
#include "./stack.hpp"

#include <optional.hpp>

struct _class;
struct method;

struct execution_context {
	method& method;
	optional<execution_context&> previous;

	nuint frames_until_end() const {
		if(previous.has_no_value()) {
			return 1;
		}
		return 1 + previous->frames_until_end();
	}
};
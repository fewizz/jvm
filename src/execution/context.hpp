#pragma once

#include <core/meta/elements/optional.hpp>
#include "object/reference/decl.hpp"

struct _class;
struct method;

struct execution_context {
	_class& _class;
	method& method;
	optional<execution_context&> previous;
};
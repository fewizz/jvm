#pragma once

#include <core/meta/elements/optional.hpp>

struct _class;
struct method;

struct execution_context {
	_class& _class;
	method& method;
	uint32& pc;
	optional<execution_context&> previous;
};

using exec_ctx = execution_context;
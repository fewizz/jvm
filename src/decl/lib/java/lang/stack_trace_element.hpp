#pragma once

#include "class.hpp"
#include "method.hpp"
#include "reference.hpp"

#include <optional.hpp>

static optional<_class&> stack_trace_element_class;
static optional<method&> stack_trace_element_constructor;

template<basic_range ClassName, basic_range MethodName>
static inline reference create_stack_trace_element(
	ClassName&& class_name, MethodName&& method_name
);
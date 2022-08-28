#pragma once

#include "execution/stack.hpp"
#include "method.hpp"

using arguments_span = span<stack_entry, uint16>;

static optional<stack_entry> execute(method& m, arguments_span args = {});

inline void invoke_dynamic(
	class_file::constant::invoke_dynamic_index ref_index,
	_class& c, stack& stack
);

inline void invoke_static(
	class_file::constant::method_ref_index ref_index, _class& c, stack& stack
);
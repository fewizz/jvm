#pragma once

#include "decl/class.hpp"

#include <optional.hpp>

static optional<_class&> method_handle_class;

inline layout::position method_handle_function_ptr_position;

inline void method_handle_invoke_exact(reference mh_ref, nuint args_beginning);
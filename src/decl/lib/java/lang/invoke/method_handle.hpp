#pragma once

#include "decl/class.hpp"

#include <optional.hpp>

static optional<_class&> method_handle_class;

inline instance_method_index method_handle_invoke_exact_ptr_index;

inline void method_handle_invoke_exact(reference mh_ref, nuint args_beginning);
#pragma once

#include "class.hpp"
#include "execute.hpp"

#include <class_file/constant.hpp>

#include <optional.hpp>

static optional<_class&> method_handle_class;
inline layout::position method_handle_member_instance_field_position;
inline layout::position method_handle_kind_instance_field_position;

void method_handle_invoke_exact(
	reference ref, parameters_count
);

reference create_method_handle_invoke_static(method&);
reference create_method_handle_invoke_virtual(method&);
reference create_method_handle_invoke_special(method&);
reference create_method_handle_new_invoke_special(method&);
reference create_method_handle_new_invoke_interface(method&);
#pragma once

#include "method.hpp"
#include "reference.hpp"

[[nodiscard]] static optional<reference> try_native_interface_call(
	native_function_ptr ptr, method& m
);
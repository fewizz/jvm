#pragma once

#include "class.hpp"
#include "class/load.hpp"
#include <core/optional.hpp>

static optional<_class&> buffer_class;

static inline void init_java_nio_buffer() {
	buffer_class = load_class(c_string{ "java/nio/Buffer" });
}
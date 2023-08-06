#pragma once

#include <optional.hpp>
#include <c_string.hpp>

#include <unicode/utf8.hpp>

static optional<c_string_of_known_size<utf8::unit>>
	executable_path,
	lib_path;
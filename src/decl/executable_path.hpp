#pragma once

#include <optional.hpp>
#include <c_string.hpp>
#include <span.hpp>

#include <unicode/utf8.hpp>

static optional<span<const utf8::unit>> executable_path;
static optional<span<const utf8::unit>> lib_path;
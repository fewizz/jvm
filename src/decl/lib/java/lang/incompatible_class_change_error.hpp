#pragma once

#include "decl/reference.hpp"
#include "decl/lib/java/lang/string.hpp"

#include <expected.hpp>

[[nodiscard]] inline expected<reference, reference>
try_create_incompatible_class_change_error();

[[nodiscard]] inline expected<reference, reference>
try_create_incompatible_class_change_error(j::string& str);
#pragma once

#include "decl/reference.hpp"

[[nodiscard]] inline expected<reference, reference>
try_create_array_store_exception();
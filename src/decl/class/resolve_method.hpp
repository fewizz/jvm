#pragma once

#include "class.hpp"

/* symbolic reference from D to a method in a class C is already resolved */
template<basic_range Name, basic_range Descriptor>
[[nodiscard]] expected<method&, reference>
try_resolve_method(c& d, c& c, Name&& name, Descriptor&& descriptor);
#pragma once

#include "class.hpp"

/* symbolic reference from D to a method in a class C is already resolved */
template<basic_range Name, basic_range Descriptor>
method& resolve_method(_class& c, Name&& name, Descriptor&& descriptor);
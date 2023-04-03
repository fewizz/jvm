#pragma once

#include "../class.hpp"
#include <range.hpp>

/* The following steps are used to derive a nonarray class or interface C
   denoted by N from a purported representation in class file format using the
   class loader L. */
template<basic_range Name>
static _class& define_class(
	Name&& n,
	posix::memory_for_range_of<uint8> bytes,
	reference definig_loader
);
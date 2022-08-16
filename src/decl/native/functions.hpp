#pragma once

#include "class.hpp"
#include "native/function.hpp"
#include "alloc.hpp"

#include <memory_list.hpp>
#include <range.hpp>

static struct native_functoins :
	memory_list<native_function, uint32>
{
	using base_type = memory_list<native_function, uint32>;
	using base_type::base_type;
} native_functions{ allocate(sizeof(native_function) * 65536) };
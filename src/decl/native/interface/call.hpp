#pragma once

#include "execution/stack_entry.hpp"
#include "execute.hpp"

template<typename Descriptor>
static stack_entry native_interface_call(
	void* ptr, arguments_span args, Descriptor&&
);
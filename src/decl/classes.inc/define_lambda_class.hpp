#pragma once

#include "decl/lib/java/lang/class_loader.hpp"

#include <range.hpp>

#include <posix/memory.hpp>

template<basic_range Name, basic_range Descriptor>
expected<c&, reference> try_define_lamda_class(
	Name&& name,
	c& interface_to_implement,
	Descriptor constructor_descriptor,
	j::c_loader* defining_loader // L
) {
	
	instance_method constructor {
	};
}
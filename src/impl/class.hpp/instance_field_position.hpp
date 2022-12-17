#include "decl/class.hpp"

template<basic_range Name, basic_range Descriptor>
layout::position _class::instance_field_position(
	Name&& name, Descriptor&& descriptor
) {
	return instance_field_position(
		instance_fields().find_index_of(name, descriptor)
	);
}
#include "class.hpp"

template<range Name, range Descriptor>
static bool
try_find_instance_field_index0(
	_class& c, Name&& name, Descriptor&& descriptor, uint16& index
) {
	if(c.has_super_class()) {
		_class& super = c.super_class();
		bool result {
			try_find_instance_field_index0(super, name, descriptor, index)
		};
		if(result) {
			return true;
		}
	}
	for(instance_field& f : c.declared_instance_fields()) {
		if(
			equals(c.name(f), name) &&
			equals(c.descriptor(f), descriptor)
		) {
			return true;
		}
		++index;
	}
	return false;
}

template<range Name, range Descriptor>
optional<instance_field_index>
_class::try_find_instance_field_index(Name&& name, Descriptor&& descriptor) {
	uint16 index = 0;
	bool result {
		try_find_instance_field_index0(*this, name, descriptor, index)
	};
	if(result) {
		return instance_field_index{ index };
	}
	return elements::none{};
}
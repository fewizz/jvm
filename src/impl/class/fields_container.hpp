#include "decl/class/fields_container.hpp"

#include "class.hpp"

template<range Name, range Descriptor>
optional<field_index> fields_container::
try_find_declared_instance_field_index(
	_class& c, Name&& name, Descriptor&& desc
) {
	uint16 index = 0;
	for(field& f : *this) {
		if(equals(c.name(f), name) && equals(c.descriptor(f), desc)) {
			return field_index{ index };
		}
		++index;
	}
	return {};
}
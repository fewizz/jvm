#include "class.hpp"

static inline optional<instance_field_with_class>
try_get_instance_field0(_class& c, uint16& index) {
	if(c.has_super_class()) {
		_class& super = c.super_class();
		optional<instance_field_with_class> result {
			try_get_instance_field0(super, index)
		};
		if(result.has_value()) {
			return result;
		}
	}
	// TODO
	uint16 declared_instance_fields_count = c.declared_instance_fields().size();
	if(index >= declared_instance_fields_count) {
		index -= declared_instance_fields_count;
		return elements::none{};
	}
	return instance_field_with_class{
		c.declared_instance_fields()[index], c
	};
}

inline optional<instance_field_with_class> _class::
try_get_instance_field(instance_field_index index) {
	return try_get_instance_field0(*this, index._);
}
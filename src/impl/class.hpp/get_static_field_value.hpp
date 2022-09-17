#include "decl/class.hpp"
#include "decl/field.hpp"

#include <class_file/constant.hpp>

inline field_value& _class::get_static_field_value(
	class_file::constant::field_ref_index ref_index
) {
	if(auto& t = trampoline(ref_index); t.has_no_value()) {
		if(!t.is<field_value>()) {
			//fputs("invalid const pool entry", stderr);
			abort();
		}
		return t.get<field_value>();
	}

	namespace cc = class_file::constant;

	cc::field_ref field_ref = field_ref_constant(ref_index);
	cc::name_and_type nat {
		name_and_type_constant(field_ref.name_and_type_index)
	};
	cc::utf8 name = utf8_constant(nat.name_index);
	cc::utf8 desc = utf8_constant(nat.descriptor_index);

	_class& c = get_resolved_class(field_ref.class_index);
	c.initialise_if_need();

	declared_static_field_index index =
		c.declared_static_fields().try_find_index_of(name, desc)
		.if_has_no_value(abort)
		.value();

	field_value& value = c.declared_static_fields_values()[index];

	trampoline(ref_index) = value;
	return value;
}
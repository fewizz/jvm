#include "class.hpp"
#include "field.hpp"

#include <class_file/constant.hpp>

instance_field_index
inline _class::get_resolved_instance_field_index(
	class_file::constant::field_ref_index ref_index
) {
	if(auto& t = trampoline(ref_index); !t.is<elements::none>()) {
		if(!t.is<instance_field_index>()) {
			fputs("invalid const pool entry", stderr);
			abort();
		}
		return t.get<instance_field_index>();
	}

	namespace cc = class_file::constant;

	cc::field_ref field_ref = field_ref_constant(ref_index);
	cc::name_and_type nat = name_and_type_constant(
		field_ref.name_and_type_index
	);
	cc::utf8 field_name = utf8_constant(nat.name_index);
	cc::utf8 field_descriptor = utf8_constant(nat.descriptor_index);

	_class& c0 = get_class(field_ref.class_index);

	instance_field_index index =
		c0.find_instance_field_index(field_name, field_descriptor);
	trampoline(ref_index) = index;
	return index;
}
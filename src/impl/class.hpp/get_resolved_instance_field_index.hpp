#include "class.hpp"
#include "field.hpp"

#include <class_file/constant.hpp>

#include <tuple.hpp>

inline tuple<instance_field_index, _class&>
_class::get_resolved_instance_field_index_and_class(
	class_file::constant::field_ref_index ref_index
) {
	if(auto& t = trampoline(ref_index); t.has_value()) {
		if(!t.is<tuple<instance_field_index, _class&>>()) {
			abort();
		}
		return t.get<tuple<instance_field_index, _class&>>();
	}

	namespace cc = class_file::constant;

	cc::field_ref field_ref = field_ref_constant(ref_index);
	cc::name_and_type nat = name_and_type_constant(
		field_ref.name_and_type_index
	);
	cc::utf8 name = utf8_constant(nat.name_index);
	cc::utf8 desc = utf8_constant(nat.descriptor_index);

	_class& c = get_resolved_class(field_ref.class_index);

	instance_field_index index =
		c.instance_fields().try_find_index_of(name, desc)
		.if_has_no_value([]{ abort(); })
		.value();
	
	tuple<instance_field_index, _class&> result{ index, c };
	
	trampoline(ref_index) = result;
	return result;
}
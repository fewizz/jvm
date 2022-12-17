#include "decl/class.hpp"
#include "decl/field.hpp"

#include <class_file/constant.hpp>

inline class_and_declared_static_field_index _class::get_static_field_index(
	class_file::constant::field_ref_index ref_index
) {
	if(auto& t = trampoline(ref_index); t.has_value()) {
		if(!t.is_same_as<class_and_declared_static_field_index>()) {
			abort();
		}
		return t.get_same_as<class_and_declared_static_field_index>();
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
		.get();

	class_and_declared_static_field_index result {
		._class = c,
		.field_index = index
	};

	trampoline(ref_index) = result;
	return result;
}
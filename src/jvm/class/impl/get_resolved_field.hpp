#pragma once

#include "../declaration.hpp"
#include "../../field/declaration.hpp"
#include "../../classes/find_or_load.hpp"
#include "class/file/descriptor/reader.hpp"

instance_field_index
_class::get_resolved_instance_field_index(uint16 ref_index) {
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

	auto index0 {
		c0.try_find_instance_field_index(field_name, field_descriptor)
	};
	if(!index0.has_value()) {
		fprintf(stderr, "couldn't resolve field");
		abort();
	}
	instance_field_index index = index0.value();
	trampoline(ref_index) = index;
	return index;
}
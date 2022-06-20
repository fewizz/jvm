#pragma once

#include "../declaration.hpp"
#include "../../field/declaration.hpp"
#include "../../classes/find_or_load.hpp"
#include "class/file/descriptor/reader.hpp"

field_index _class::get_resolved_instance_field_index(uint16 ref_index) {
	if(auto& t = trampoline(ref_index); !t.is<elements::none>()) {
		if(!t.is<field_index>()) {
			fputs("invalid const pool entry", stderr);
			abort();
		}
		return t.get<field_index>();
	}

	namespace cc = class_file::constant;

	cc::field_ref field_ref = field_ref_constant(ref_index);
	cc::name_and_type nat = name_and_type_constant(
		field_ref.name_and_type_index
	);
	cc::utf8 field_name = utf8_constant(nat.name_index);
	cc::utf8 field_descriptor = utf8_constant(nat.descriptor_index);

	bool result = class_file::descriptor::read_field(
		field_descriptor.begin(),
		[&]<typename Type>(Type x) {
			if constexpr(same_as<Type, class_file::descriptor::object_type>) {
				find_or_load_class(x);
			}
			return true;
		}
	);

	if(!result) {
		fprintf(stderr, "couldn't read field descriptor");
		abort();
	}

	_class& c0 = get_class(field_ref.class_index);

	auto index0 {
		c0.try_find_instance_field_index(field_name, field_descriptor)
	};
	if(!index0.has_value()) {
		fprintf(stderr, "couldn't resolve field");
		abort();
	}
	field_index index = index0.value();
	trampoline(ref_index) = index;
	return index;
}
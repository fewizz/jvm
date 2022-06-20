#pragma once

#include "../declaration.hpp"
#include "../../field/declaration.hpp"
#include "../../classes/find_or_load.hpp"
#include "class/file/descriptor/reader.hpp"

static_field_with_class _class::get_static_field(uint16 ref_index) {
	if(auto& t = trampoline(ref_index); !t.is<elements::none>()) {
		if(!t.is<static_field_with_class>()) {
			fputs("invalid const pool entry", stderr);
			abort();
		}
		return t.get<static_field_with_class>();
	}

	namespace cc = class_file::constant;

	cc::field_ref field_ref = field_ref_constant(ref_index);
	cc::name_and_type nat {
		name_and_type_constant(field_ref.name_and_type_index)
	};
	cc::utf8 name = utf8_constant(nat.name_index);
	cc::utf8 descriptor = utf8_constant(nat.descriptor_index);

	bool result = class_file::descriptor::read_field(
		descriptor.begin(),
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
	optional<field&> field = c0.try_find_field(name, descriptor);
	if(!field.has_value()) {
		fputs("couldn't find field", stderr);
		abort();
	}
	if(!field.value().is_static()) {
		fputs("field isn't static", stderr);
		abort();
	}
	static_field_with_class sfwc{ (static_field&) field.value(), c0 };
	trampoline(ref_index) = sfwc;
	return sfwc;
}
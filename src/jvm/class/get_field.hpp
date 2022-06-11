#pragma once

#include "../field.hpp"
#include "../classes.hpp"
#include "class/file/descriptor/reader.hpp"

field& _class::get_static_field(uint16 ref_index) {
	if(auto& t = trampoline(ref_index); !t.is<elements::none>()) {
		return t.get<static_field&>();
	}

	using namespace class_file;

	auto ref = field_ref_constant(ref_index);
	auto class_info = class_constant(ref.class_index);
	auto nat = name_and_type_constant(ref.name_and_type_index);
	auto name = utf8_constant(nat.name_index);
	auto descriptor = utf8_constant(nat.descriptor_index);

	bool result = class_file::descriptor::read_field(
		descriptor.begin(),
		[&]<typename Type>(Type x) {
			if constexpr(same_as<Type, class_file::descriptor::object_type>) {
				find_or_load(x);
			}
			return true;
		}
	);

	if(!result) {
		fprintf(stderr, "couldn't read field descriptor");
		abort();
	}

	auto clss = utf8_constant(class_info.name_index);
	_class& c = find_or_load(clss);

	static_field& f = (static_field&)c.find_field(name);
	trampoline(ref_index) = f;
	return f;
}
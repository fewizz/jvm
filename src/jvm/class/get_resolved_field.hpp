#pragma once

#include "../field.hpp"
#include "../classes.hpp"
#include "class/file/descriptor/reader.hpp"

instance_field_index
_class::get_resolved_instance_field_index(uint16 ref_index) {
	if(auto& t = trampoline(ref_index); !t.is<decltype(nullptr)>()) {
		return t.get<instance_field_index>();
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

	auto class_name = utf8_constant(class_info.name_index);
	_class* other_c = &find_or_load(class_name);;
	::field* f = nullptr;

	while(true) {
		if(f = other_c->try_find_field(name, descriptor); f != nullptr) {
			break;
		}
		if(other_c->super_class_index_ == 0) {
			fprintf(stderr, "couldn't resolve field");
			abort();
		}
		other_c = &other_c->get_class(other_c->super_class_index_);
	}

	uint16 index = 0;
	for(::field& f0 : f->_class().instance_fields()) {
		if(&f0 == f) {
			trampoline(ref_index) = instance_field_index{ index };
			return { index };
		}
		++index;
	}

	fputs("couldn't create trampoline for instance field", stderr);
	abort();
}
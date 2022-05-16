#pragma once

#include "../method.hpp"

method& _class::get_resolved_method(uint16 ref_index) {
	if(auto& t = trampoline(ref_index); !t.is<decltype(nullptr)>()) {
		return t.get<method&>();
	}

	auto ref = method_ref_constant(ref_index);
	auto class_info = class_constant(ref.class_index);
	auto nat = name_and_type_constant(ref.name_and_type_index);
	auto name = utf8_constant(nat.name_index);
	auto descriptor = utf8_constant(nat.descriptor_index);

	class_file::descriptor::method_reader params{ descriptor.begin() };
	auto [ret_reader, result0] = params([&]<typename Type>(Type x) {
		if constexpr(same_as<Type, class_file::descriptor::object_type>) {
			find_or_load(x);
		}
		return true;
	});
	if(!result0) {
		fprintf(stderr, "couldn't read method descriptor parameters");
		abort();
	}

	auto [end, result1] = ret_reader([&]<typename Type>(Type x) {
		if constexpr(same_as<Type, class_file::descriptor::object_type>) {
			find_or_load(x);
		}
		return true;
	});

	if(!result1) {
		fprintf(stderr, "couldn't read method descriptor return type");
		abort();
	}

	auto class_name = utf8_constant(class_info.name_index);
	_class* other_c = &find_or_load(class_name);;
	method* m = nullptr;

	while(true) {
		if(m = other_c->try_find_method(name, descriptor); m != nullptr) {
			break;
		}
		if(other_c->super_class_index_ == 0) {
			fprintf(stderr, "couldn't resolve method");
			abort();
		}
		other_c = &other_c->get_class(other_c->super_class_index_);
	}

	method& r = *m;
	trampoline(ref_index) = r;
	return r;
}
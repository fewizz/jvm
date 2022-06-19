#pragma once

#include "../declaration.hpp"
#include "../../method/declaration.hpp"
#include "../../classes/find_or_load.hpp"
#include "class/file/descriptor/reader.hpp"

method_with_class _class::get_resolved_method(uint16 ref_index) {
	if(auto& t = trampoline(ref_index); !t.is<elements::none>()) {
		if(!t.is<method_with_class>()) {
			fputs("invalid const pool entry", stderr);
			abort();
		}
		return t.get<method_with_class>();
	}

	namespace cc = class_file::constant;

	cc::method_ref method_ref = method_ref_constant(ref_index);
	cc::_class _c = class_constant(method_ref.class_index);
	cc::name_and_type nat {
		name_and_type_constant(method_ref.name_and_type_index)
	};
	cc::utf8 name = utf8_constant(nat.name_index);
	cc::utf8 descriptor = utf8_constant(nat.descriptor_index);

	class_file::descriptor::method_reader params{ descriptor.begin() };
	auto [ret_reader, result0] = params([&]<typename Type>(Type x) {
		if constexpr(same_as<Type, class_file::descriptor::object_type>) {
			find_or_load_class(x);
		}
		return true;
	});
	if(!result0) {
		fprintf(stderr, "couldn't read method descriptor parameters");
		abort();
	}

	auto [end, result1] = ret_reader([&]<typename Type>(Type x) {
		if constexpr(same_as<Type, class_file::descriptor::object_type>) {
			find_or_load_class(x);
		}
		return true;
	});

	if(!result1) {
		fprintf(stderr, "couldn't read method descriptor return type");
		abort();
	}

	auto class_name = utf8_constant(_c.name_index);
	optional<_class&> other_c { find_or_load_class(class_name) };
	optional<method&> m{};

	while(true) {
		if(
			m = other_c->try_find_method(name, descriptor);
			m.has_value()
		) {
			break;
		}
		if(!other_c->has_super_class()) {
			fprintf(stderr, "couldn't resolve method");
			abort();
		}
		other_c = other_c->super_class();
	}

	method_with_class mwc{ m.value(), other_c.value() };
	trampoline(ref_index) = mwc;
	return mwc;
}
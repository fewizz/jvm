#pragma once

#include "../decl.hpp"
#include "../../method/decl.hpp"
#include "../../classes/find_or_load.hpp"
#include "class/file/descriptor/reader.hpp"

method_with_class _class::get_static_method(uint16 ref_index) {
	if(auto& t = trampoline(ref_index); !t.is<elements::none>()) {
		if(!t.is<method_with_class>()) {
			fputs("invalid const pool entry", stderr);
			abort();
		}
		return t.get<method_with_class>();
	}

	namespace cc = class_file::constant;

	cc::method_ref method_ref = method_ref_constant(ref_index);
	cc::name_and_type nat {
		name_and_type_constant(method_ref.name_and_type_index)
	};
	cc::utf8 method_name = utf8_constant(nat.name_index);
	cc::utf8 method_desc = utf8_constant(nat.descriptor_index);

	class_file::descriptor::method_reader params{ method_desc.begin() };
	auto [ret_reader, result0] = params([&]<typename Type>(Type x) {
		if constexpr(same_as<Type, class_file::descriptor::object_type>) {
			find_or_load_class(x);
		}
		return true;
	});
	if(!result0) {
		fprintf(stderr, "couldn't read method's descriptor parameters");
		abort();
	}

	auto [end, result1] = ret_reader([&]<typename Type>(Type x) {
		if constexpr(same_as<Type, class_file::descriptor::object_type>) {
			find_or_load_class(x);
		}
		return true;
	});

	if(!result1) {
		fprintf(stderr, "couldn't read method's descriptor return type: ");
		fwrite(method_name.data(), 1, method_desc.size(), stderr);
		fwrite(method_desc.data(), 1, method_desc.size(), stderr);
		abort();
	}

	_class& c0 = get_class(method_ref.class_index);
	c0.initialise_if_need();
	optional<method&> m0 = c0.try_find_method(method_name, method_desc);

	if(!m0.has_value()) {
		fputs("could find static method", stderr);
		abort();
	}

	method_with_class mwc{ m0.value(), c0 };
	trampoline(ref_index) = mwc;
	return mwc;
}
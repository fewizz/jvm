#include "class.hpp"
#include "method.hpp"

#include <class_file/constant.hpp>

inline method_with_class _class::get_resolved_method(
	class_file::constant::method_ref_index ref_index
) {
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
	cc::utf8 name = utf8_constant(nat.name_index);
	cc::utf8 descriptor = utf8_constant(nat.descriptor_index);

	optional<_class&> c0 = get_class(method_ref.class_index);
	optional<method&> m0{};

	while(true) {
		if(
			m0 = c0->try_find_method(name, descriptor);
			m0.has_value()
		) {
			break;
		}
		if(!c0->has_super_class()) {
			fprintf(stderr, "couldn't resolve method");
			abort();
		}
		c0 = c0->super_class();
	}

	method_with_class mwc{ m0.value(), c0.value() };
	trampoline(ref_index) = mwc;
	return mwc;
}
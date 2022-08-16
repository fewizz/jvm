#include "decl/class.hpp"
#include "decl/method.hpp"

#include <class_file/constant.hpp>

inline method& _class::get_static_method(
	class_file::constant::method_ref_index ref_index
) {
	if(auto& t = trampoline(ref_index); !t.is<elements::none>()) {
		if(!t.is<method&>()) {
			abort();
		}
		return t.get<method&>();
	}

	namespace cc = class_file::constant;

	cc::method_ref method_ref = method_ref_constant(ref_index);
	cc::name_and_type nat {
		name_and_type_constant(method_ref.name_and_type_index)
	};
	cc::utf8 name = utf8_constant(nat.name_index);
	cc::utf8 desc = utf8_constant(nat.descriptor_index);

	_class& c = get_class(method_ref.class_index);
	c.initialise_if_need();
	method& m =
		c.declared_static_methods().try_find(name, desc)
		.if_no_value([]{
			fputs("could find static method", stderr);
			abort();
		}).value();

	trampoline(ref_index) = m;
	return m;
}
#include "decl/class.hpp"
#include "decl/method.hpp"

#include <class_file/constant.hpp>

#include <loop_action.hpp>

inline instance_method_index _class::get_resolved_instance_method_index(
	class_file::constant::method_ref_index ref_index
) {
	if(auto& t = trampoline(ref_index); !t.is<elements::none>()) {
		if(!t.is<instance_method_index>()) {
			abort();
		}
		return t.get<instance_method_index>();
	}

	namespace cc = class_file::constant;

	cc::method_ref method_ref = method_ref_constant(ref_index);
	cc::name_and_type nat {
		name_and_type_constant(method_ref.name_and_type_index)
	};
	cc::utf8 name = utf8_constant(nat.name_index);
	cc::utf8 descriptor = utf8_constant(nat.descriptor_index);

	_class& referenced_class = get_class(method_ref.class_index);
	optional<method&> m =
		referenced_class.instance_methods().try_find(name, descriptor);

	instance_method_index index =
		range{ referenced_class.instance_methods() }
		.try_find_index_of_first_satisfying([&](method& m0) {
			return &m.value() == &m0;
		}).value();

	trampoline(ref_index) = index;
	return index;
}
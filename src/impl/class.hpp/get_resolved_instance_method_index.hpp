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

	// "If the maximally-specific superinterface methods of C for the name and
	// descriptor specified by the method reference include exactly one method
	// that does not have its ACC_ABSTRACT flag set, then this method is chosen
	// and method lookup succeeds."
	if(!m.has_value()) {
		referenced_class
		.for_each_maximally_specific_super_interface_instance_method(
			name, descriptor,
			[&](method& m0) {
				if(!m0.access_flags().abstract()) {
					return loop_action::stop;
				}
				m = m0;
				return loop_action::next;
			}
		);
	}

	// "Otherwise, if any superinterface of C declares a method with the name
	// and descriptor specified by the method reference that has neither
	// its ACC_PRIVATE flag nor its ACC_STATIC flag set, one of these is
	// arbitrarily chosen and method lookup succeeds."
	if(!m.has_value()) {
		referenced_class.for_each_super_interface([&](_class& i) {
			for(method& m0 : i.declared_instance_methods()) {
				if(
					!m0.access_flags()._private() &&
					range{ m0.name() }.equals_to(name) &&
					range{ m0.descriptor() }.equals_to(descriptor)
				) {
					m = m0;
					return loop_action::stop;
				}
			}
			return loop_action::next;
		});
	}

	if(!m.has_value()) {
		abort();
	}

	instance_method_index index =
		range{ referenced_class.instance_methods() }
		.try_find_index_of_first_satisfying([&](method& m0) {
			return &m.value() == &m0;
		}).value();

	trampoline(ref_index) = index;
	return index;
}
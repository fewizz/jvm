#include "class.hpp"
#include "method.hpp"

#include <class_file/constant.hpp>

#include <core/loop_action.hpp>

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

	_class& c = get_class(method_ref.class_index);
	optional<_class&> c0 = c;
	optional<method&> m{};

	while(true) {
		if(m = c0->try_find_method(name, descriptor); m.has_value()) {
			break;
		}
		if(!c0->has_super_class()) {
			break;
		}
		c0 = c0->super_class();
	}

	// "If the maximally-specific superinterface methods of C for the name and
	// descriptor specified by the method reference include exactly one method
	// that does not have its ACC_ABSTRACT flag set, then this method is chosen
	// and method lookup succeeds."
	if(!m.has_value()) {
		c.for_each_maximally_specific_superinterface_method(name, descriptor,
			[&](method_with_class mwc) {
				if(!mwc.method().access_flags().abstract()) {
					return loop_action::stop;
				}
				return loop_action::next;
			}
		);
	}

	// "Otherwise, if any superinterface of C declares a method with the name
	// and descriptor specified by the method reference that has neither
	// its ACC_PRIVATE flag nor its ACC_STATIC flag set, one of these is
	// arbitrarily chosen and method lookup succeeds.""
	if(!m.has_value()) {
		c.for_each_superinterface([&](_class& i) {
			
		})
	}

	method_with_class mwc{ m.value(), c.value() };
	trampoline(ref_index) = mwc;
	return mwc;
}
#include "decl/class.hpp"
#include "decl/method.hpp"
#include "decl/class/has_name_and_descriptor_equal_to.hpp"
#include "decl/lib/java/lang/object.hpp"

#include <loop_action.hpp>

method& _class::resolve_interface_method(
	class_file::constant::interface_method_ref ref
) {
	/* "To resolve an unresolved symbolic reference from D to an interface
	    method in an interface C, the symbolic reference to C given by the
	    interface method reference is first resolved (§5.4.3.1)." */
	_class& c = get_resolved_class(ref.interface_index);
	auto nat = name_and_type_constant(ref.name_and_type_index);
	auto name = utf8_constant(nat.name_index);
	auto descriptor = utf8_constant(nat.descriptor_index);

	/* "1. If C is not an interface, interface method resolution throws an
	    IncompatibleClassChangeError."*/ // TODO

	/* "2. Otherwise, if C declares a method with the name and descriptor
	    specified by the interface method reference, method lookup
	    succeeds." */
	optional<method&> m =
		c.declared_instance_methods().try_find(name, descriptor);
	if(m.has_value()) {
		return m.get();
	}

	/* "3. Otherwise, if the class Object declares a method with the name
	    and descriptor specified by the interface method reference, which has
	    its ACC_PUBLIC flag set and does not have its ACC_STATIC flag set,
	    method lookup succeeds." */
	m = object_class->declared_methods().try_find_first_satisfying(
		[&](method& m) {
			return
				m.access_flags()._public &&
				!m.access_flags()._static &&
				has_name_and_descriptor_equal_to{name, descriptor}(m);
		}
	);
	if(m.has_value()) {
		return m.get();
	}

	/* "4. Otherwise, if the maximally-specific superinterface methods
	    (§5.4.3.3) of C for the name and descriptor specified by the method
	    reference include exactly one method that does not have its ACC_ABSTRACT
	    flag set, then this method is chosen and method lookup succeeds." */
	c.for_each_maximally_specific_super_interface_instance_method(
		name, descriptor,
		[&](method& m0) {
			if(!m0.access_flags().abstract) {
				m = m0;
				return loop_action::stop;
			}
			return loop_action::next;
		}
	);
	if(m.has_value()) {
		return m.get();
	}

	/* "5. Otherwise, if any superinterface of C declares a method with the
	    name and descriptor specified by the method reference that has neither its
	    ACC_PRIVATE flag nor its ACC_STATIC flag set, one of these is arbitrarily
	    chosen and method lookup succeeds. */
	c.for_each_super_interface([&](_class& i) {
		for(method& m0 : i.declared_instance_methods()) {
			if(
				!m0.access_flags()._private &&
				has_name_and_descriptor_equal_to{ name, descriptor}(m0)
			) {
				m = m0;
				return loop_action::stop;
			}
		}
		return loop_action::next;
	});
	if(m.has_value()) {
		return m.get();
	}

	// "6. Otherwise, method lookup fails."
	abort();
}
#include "decl/class.hpp"
#include "decl/method.hpp"
#include "decl/class/has_name_and_descriptor_equal_to.hpp"
#include "decl/class/access_control.hpp"
#include "decl/lib/java/lang/object.hpp"
#include "decl/lib/java/lang/incompatible_class_change_error.hpp"
#include "decl/lib/java/lang/no_such_method_error.hpp"

#include <loop_action.hpp>

template<basic_range Name, basic_range Descriptor>
inline expected<method&, reference> try_resolve_interface_method(
	_class& d, _class& c, Name&& name, Descriptor&& descriptor
) {
	/* 1. If C is not an interface, interface method resolution throws an
	      IncompatibleClassChangeError. */
	if(!c.is_interface()) {
		expected<reference, reference> possible_icce
			=try_create_incompatible_class_change_error();
		return unexpected { move(possible_icce.get()) };
	}

	/* 2. Otherwise, if C declares a method with the name and descriptor
	      specified by the interface method reference, method lookup
	      succeeds. */
	optional<method&> possible_m =
		c.declared_instance_methods().try_find(name, descriptor);

	/* 3. Otherwise, if the class Object declares a method with the name
	      and descriptor specified by the interface method reference, which has
	      its ACC_PUBLIC flag set and does not have its ACC_STATIC flag set,
	      method lookup succeeds. */
	if(!possible_m.has_value()) {
		possible_m = object_class->declared_methods().try_find_first_satisfying(
			[&](method& m) {
				return
					m.is_public() && !m.is_static() &&
					has_name_and_descriptor_equal_to{name, descriptor}(m);
			}
		);
	}

	/* 4. Otherwise, if the maximally-specific superinterface methods
	      (§5.4.3.3) of C for the name and descriptor specified by the method
	      reference include exactly one method that does not have its
	      ACC_ABSTRACT flag set, then this method is chosen and method lookup
	      succeeds. */
	if(!possible_m.has_value()) {
		c.for_each_maximally_specific_super_interface_instance_method(
			name, descriptor,
			[&](method& m0) {
				if(!m0.is_abstract()) {
					if(possible_m.has_value()) { // more than one
						possible_m = {};
						return loop_action::stop;
					}
					possible_m = m0;
				}
				return loop_action::next;
			}
		);
	}

	/* 5. Otherwise, if any superinterface of C declares a method with the name
	      and descriptor specified by the method reference that has neither its
	      ACC_PRIVATE flag nor its ACC_STATIC flag set, one of these is
	      arbitrarily chosen and method lookup succeeds. */
	if(!possible_m.has_value()) {
		c.for_each_super_interface([&](_class& i) {
			for(method& m0 : i.declared_instance_methods()) {
				if(
					!m0.is_private() &&
					has_name_and_descriptor_equal_to{ name, descriptor}(m0)
				) {
					possible_m = m0;
					return loop_action::stop;
				}
			}
			return loop_action::next;
		});
	}

	// 6. Otherwise, method lookup fails.

	/* The result of interface method resolution is determined as follows: */
	/* • If method lookup failed, interface method resolution throws a
	     NoSuchMethodError */
	if(!possible_m.has_value()) {
		expected<reference, reference> possible_nsme
			= try_create_no_such_method_error();
		
		return unexpected { move(possible_nsme.get())};
	}

	method& m = possible_m.get();

	/* • Otherwise, method lookup succeeded. Access control is applied for the
	     access from D to the method which is the result of method lookup
	     (§5.4.4). */
	optional<reference> possible_error = access_control(d, m);
	if(possible_error.has_value()) {
		/* - If access control failed, interface method resolution fails for the
		     same reason. */
		return unexpected{ move(possible_error.get()) };
	}

	/* - Otherwise, access control succeeded. Loading constraints are imposed,
	     as follows. */ // TODO

	return m;
}

inline expected<method&, reference> _class::try_resolve_interface_method(
	class_file::constant::interface_method_ref ref
) {
	/* To resolve an unresolved symbolic reference from D to an interface
	   method in an interface C, the symbolic reference to C given by the
	   interface method reference is first resolved (§5.4.3.1). */
	expected<_class&, reference> possible_c
		= try_get_resolved_class(ref.class_index);

	if(possible_c.is_unexpected()) {
		return unexpected{ possible_c.get_unexpected() };
	}

	_class& c = possible_c.get_expected();

	auto nat = name_and_type_constant(ref.name_and_type_index);
	auto name = utf8_constant(nat.name_index);
	auto descriptor = utf8_constant(nat.descriptor_index);

	return ::try_resolve_interface_method(*this, c, name, descriptor);
}
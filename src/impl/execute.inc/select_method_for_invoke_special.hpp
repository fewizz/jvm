#include "decl/execute.hpp"
#include "decl/class.hpp"
#include "decl/lib/java/lang/object.hpp"

#include <loop_action.hpp>

[[nodiscard]] inline optional<instance_method&>
select_method_for_invoke_special(
	c& current, c& referenced_class, method& resolved_method
) {
	/* If all of the following are true, let C be the direct superclass of
	   the current class: */
	bool c_is_direct_super_class =
		/* The resolved method is not an instance initialization method
		   (§2.9.1). */
		!resolved_method.is_instance_initialisation() &&
		/* The symbolic reference names a class (not an interface), and that
		   class is a superclass of the current class. */
		!referenced_class.is_interface() &&
		 referenced_class.is_sub_of(current);// &&
		/* The ACC_SUPER flag is set for the class file (§4.1). */
		/* always set: In Java SE 8 and above, the Java Virtual Machine
		   considers the ACC_SUPER flag to be set in every class file,
		   regardless of the actual value of the flag in the class file and
		   the version of the class file */

	c& c =
		c_is_direct_super_class ? current.super() :
		/* Otherwise, let C be the class or interface named by the symbolic
		   reference. */
		referenced_class;

	/* The actual method to be invoked is selected by the following lookup
	   procedure: */
	/* 1. If C contains a declaration for an instance method with the same
	      name and descriptor as the resolved method, then it is the method to
	      be invoked. */
	/* 2. Otherwise, if C is a class and has a superclass, a search for a
	      declaration of an instance method with the same name and descriptor
	      as the resolved method is performed, starting with the direct
	      superclass of C and continuing with the direct superclass of that
	      class, and so forth, until a match is found or no further
	      superclasses exist. If a match is found, then it is the method to be
	      invoked. */
	optional<instance_method&> possible_m;
	{
		::c* c0 = &c;
		do {
			possible_m = c0->declared_instance_methods().try_find(
				resolved_method.name(), resolved_method.descriptor()
			);
			if(possible_m.has_value()) {
				break;
			}
		} while(c0->has_super() ? c0 = &c0->super(), true : false);
	}

	/* 3. Otherwise, if C is an interface and the class Object contains a
	      declaration of a public instance method with the same name and
	      descriptor as the resolved method, then it is the method to be
	      invoked. */
	if(!possible_m.has_value() && c.is_interface()) {
		object_class->declared_instance_methods().try_find(
			resolved_method.name(), resolved_method.descriptor()
		).if_has_value([&](instance_method& m) {
			if(m.is_public()) {
				possible_m = m;
			}
		});
	}

	if(!possible_m.has_value()) {
		/* 4. Otherwise, if there is exactly one maximally-specific method
		   (§5.4.3.3) in the superinterfaces of C that matches the resolved
		   method's name and descriptor and is not abstract, then it is the
		   method to be invoked. */
		c.for_each_maximally_specific_super_interface_instance_method(
			resolved_method.name(), resolved_method.descriptor(),
			[&](instance_method& m) {
				if(!m.is_abstract()) {
					if(possible_m.has_value()) { // more than one
						possible_m = {};
						return loop_action::stop;
					}
					possible_m = m;
				}
				return loop_action::next;
			}
		);
	}

	return possible_m;
}
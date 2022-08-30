#include "decl/execute.hpp"
#include "decl/execution/info.hpp"
#include "decl/execution/stack.hpp"
#include "decl/class.hpp"
#include "decl/method.hpp"
#include "decl/lib/java/lang/object.hpp"

#include <loop_action.hpp>

inline void invoke_special(
	class_file::constant::method_ref_index ref_index, _class& on, stack& stack
) {
	namespace cc = class_file::constant;

	cc::method_ref method_ref = on.method_ref_constant(ref_index);
	cc::name_and_type nat {
		on.name_and_type_constant(method_ref.name_and_type_index)
	};
	
	cc::utf8 method_desc = on.utf8_constant(nat.descriptor_index);

	if(info) {
		tabs(); fputs("invoke_special ", stderr);
		cc::_class _c = on.class_constant(method_ref.class_index);
		cc::utf8 class_name = on.utf8_constant(_c.name_index);
		fwrite(class_name.elements_ptr(), 1, class_name.size(), stderr);
		fputc('.', stderr);
		cc::utf8 method_name = on.utf8_constant(nat.name_index);
		fwrite(method_name.elements_ptr(), 1, method_name.size(), stderr);
		fwrite(method_desc.elements_ptr(), 1, method_desc.size(), stderr);
		fputc('\n', stderr);
	}

	method& resolved_method = on.get_resolved_method(ref_index);
	_class& referenced_class = on.get_class(method_ref.class_index);

	/* "If all of the following are true, let C be the direct superclass of
	    the current class:" */
	bool c_is_direct_super_class =
		/*    "The resolved method is not an instance initialization method
		       (§2.9.1)." */
		!resolved_method.is_instance_initialisation() &&
		/*    "The symbolic reference names a class (not an interface), and that
		       class is a superclass of the current class." */
		!referenced_class.is_interface() && on.is_sub_of(on) &&
		/*    "The ACC_SUPER flag is set for the class file (§4.1)." */
		/*    always set: "In Java SE 8 and above, the Java Virtual Machine
		      considers the ACC_SUPER flag to be set in every class file,
		      regardless of the actual value of the flag in the class file and
		      the version of the class file" */
		on.access_flags().super();

	_class& c =
		c_is_direct_super_class ? on.super() :
		/* "Otherwise, let C be the class or interface named by the symbolic
		    reference.s" */
		referenced_class;

	/* "The actual method to be invoked is selected by the following lookup
	    procedure:" */
	/*    "1. If C contains a declaration for an instance method with the same
	       name and descriptor as the resolved method, then it is the method to
	       be invoked." */
	/*    "2. "Otherwise, if C is a class and has a superclass, a search for a
	       declaration of an instance method with the same name and descriptor
	       as the resolved method is performed, starting with the direct
	       superclass of C and continuing with the direct superclass of that
	       class, and so forth, until a match is found or no further
	       superclasses exist. If a match is found, then it is the method to be
	       invoked." */
	optional<method&> m = c.instance_methods().try_find(
		resolved_method.name(), resolved_method.descriptor()
	);

	if(!m.has_value() && c.is_interface()) {
		/* "3. Otherwise, if C is an interface and the class Object contains a
		    declaration of a public instance method with the same name and
		    descriptor as the resolved method, then it is the method to be
		    invoked." */
		m = object_class->declared_instance_methods().try_find(
			resolved_method.name(), resolved_method.descriptor()
		);
	}

	if(!m.has_value()) {
		/* "4. Otherwise, if there is exactly one maximally-specific method
		    (§5.4.3.3) in the superinterfaces of C that matches the resolved
		    method's name and descriptor and is not abstract, then it is the
		    method to be invoked." */
		c.for_each_maximally_specific_super_interface_instance_method(
			resolved_method.name(), resolved_method.descriptor(),
			[&](method& m0) {
				if(!m0.access_flags().abstract()) {
					m = m0;
					return loop_action::stop;
				}
				return loop_action::next;
			}
		);
	}

	if(!m.has_value()) {
		abort();
	}

	uint8 args_count = resolved_method.parameters_count();
	++args_count; // this
	optional<stack_entry> result = execute(
		m.value(),
		arguments_span {
			stack.iterator() + stack.size() - args_count, args_count
		}
	);

	stack.pop_back(args_count);

	result.if_has_value([&](stack_entry& value) {
		stack.emplace_back(move(value));
	});

}
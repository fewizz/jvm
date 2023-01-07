#include "decl/class/resolve_method.hpp"
#include "decl/method.hpp"
#include "decl/class.hpp"
#include "decl/class/has_name_and_descriptor_equal_to.hpp"
#include "decl/lib/java/lang/object.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"

#include <loop_action.hpp>

inline method& _class::resolve_method(
	class_file::constant::method_ref ref
) {
	/* "To resolve an unresolved symbolic reference from D to a method in a
	    class C, the symbolic reference to C given by the method reference is
	    first resolved (§5.4.3.1)" */
	_class& c = get_resolved_class(ref.class_index);
	auto nat = name_and_type_constant(ref.name_and_type_index);
	auto name = utf8_constant(nat.name_index);
	auto descriptor = utf8_constant(nat.descriptor_index);
	return ::resolve_method(c, name, descriptor);
}

/* 2. Otherwise, method resolution attempts to locate the referenced method in C
      and its superclasses: */
template<basic_range Name, basic_range Descriptor>
inline optional<method&> try_method_resolution_step_2(
	_class& c, Name&& name, Descriptor&& descriptor
) {
	/*    "If C declares exactly one method with the name specified by the
	       method reference, and the declaration is a signature polymorphic
	       method (§2.9.3), then method lookup succeeds. All the class names
	       mentioned in the descriptor are resolved"*/
	if(
		&c == method_handle_class.ptr()
	){
		auto possible_poly_methods = c.declared_methods().filter_view(
			[&](method& m) {
				return
					m.access_flags().varargs &&
					m.access_flags().native &&
					m.name().has_equal_size_and_elements(name) &&
					m.descriptor().has_equal_size_and_elements(
						c_string{ "([Ljava/lang/Object;)Ljava/lang/Object;" }
					);
			}
		);
		optional<method&> first_poly_method;
		nuint count = 0;
		for(method& m : possible_poly_methods) {
			if(count == 0) first_poly_method = m;
			++count;
		}
		if(count == 1) {
			return first_poly_method.get();
		}
	}

	/*    "Otherwise, if C declares a method with the name and descriptor
	       specified by the method reference, method lookup succeeds."*/
	if(
		optional<method&> m = c.declared_methods().try_find(name, descriptor);
		m.has_value()
	) {
		return m;
	}

	/*    "Otherwise, if C has a superclass, step 2 of method resolution is
	       recursively invoked on the direct superclass of C." */
	if(c.has_super()) {
		return try_method_resolution_step_2(c.super(), name, descriptor);
	}
	return {};
}

/* symbolic reference from D to a method in a class C is already resolved */
template<basic_range Name, basic_range Descriptor>
method& resolve_method(_class& c, Name&& name, Descriptor&& descriptor) {
	/* "1. If C is an interface, method resolution throws an
	    IncompatibleClassChangeError."*/ // TODO

	/*    "Otherwise, if C has a superclass, step 2 of method resolution is
	       recursively invoked on the direct superclass of C." */
	optional<method&> m = try_method_resolution_step_2(c, name, descriptor);
	if(m.has_value()) {
		return m.get();
	}

	/* "3. Otherwise, method resolution attempts to locate the referenced method
	    in the superinterfaces of the specified class C:"*/
	/*"    If the maximally-specific superinterface methods of C for the name
	       and descriptor specified by the method reference include exactly one
	       method that does not have its ACC_ABSTRACT flag set, then this method
	       is chosen and method lookup succeeds."*/
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

	/*    "Otherwise, if any superinterface of C declares a method with the name
	       and descriptor specified by the method reference that has neither
	       its ACC_PRIVATE flag nor its ACC_STATIC flag set, one of these is
	       arbitrarily chosen and method lookup succeeds."*/
	c.for_each_super_interface([&](_class& i) {
		for(method& m0 : i.declared_instance_methods()) {
			if(
				!m0.access_flags()._private &&
				has_name_and_descriptor_equal_to{ name, descriptor }(m0)
			) {
				m = m0;
				return loop_action::stop;
			}
		}
		return loop_action::next;
	});

	//    "Otherwise, method lookup fails.""
	posix::abort();
}
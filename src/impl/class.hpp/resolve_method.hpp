#include "decl/class/resolve_method.hpp"

#include "decl/method.hpp"
#include "decl/class.hpp"
#include "decl/class/has_name_and_descriptor_equal_to.hpp"
#include "decl/class/access_control.hpp"
#include "decl/lib/java/lang/object.hpp"
#include "decl/lib/java/lang/incompatible_class_change_error.hpp"
#include "decl/lib/java/lang/illegal_access_error.hpp"
#include "decl/lib/java/lang/no_such_method_error.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/invoke/var_handle.hpp"

#include <loop_action.hpp>

/* 2. Otherwise, method resolution attempts to locate the referenced method in C
      and its superclasses: */
template<basic_range Name, basic_range Descriptor>
inline expected<optional<method&>, reference> try_method_resolution_step_2(
	_class& c, Name&& name, Descriptor&& descriptor
) {
	/*    If C declares exactly one method with the name specified by the
	      method reference, and the declaration is a signature polymorphic
	      method (§2.9.3), then method lookup succeeds. All the class names
	      mentioned in the descriptor are resolved */
	auto methods_with_same_name
		= c.declared_methods().filter_view([&](method& m) {
			return m.name().has_equal_size_and_elements(name);
		});
	
	bool only_one_method_with_same_name
		= methods_with_same_name.get_or_compute_size() == 1;

	if(only_one_method_with_same_name) {
		method& m = *methods_with_same_name.iterator();
		
		bool is_signature_polymorphic = m.is_signature_polymorphic();
		
		if(is_signature_polymorphic) {
			reference thrown;

			class_file::method_descriptor::try_read_parameter_and_return_types(
				descriptor.iterator(),
				[&]<typename ParamType>(ParamType p) {
					expected<_class&, reference> possible_c
						= try_resolve_class_from_type(c, p);
					if(possible_c.is_unexpected()) {
						thrown = move(possible_c.get_unexpected());
						return;
					}
				},
				[&]<typename ReturnType>(ReturnType r) {
					if(!thrown.is_null()) return;
					expected<_class&, reference> possible_c
						= try_resolve_class_from_type(c, r);
					if(possible_c.is_unexpected()) {
						thrown = move(possible_c.get_unexpected());
						return;
					}
				},
				[](auto) { posix::abort(); }
			);

			if(!thrown.is_null()) {
				return unexpected{ move(thrown) };
			}

			return m;
		}
	}

	/*    Otherwise, if C declares a method with the name and descriptor
	      specified by the method reference, method lookup succeeds. */
	optional<method&> m = c.declared_methods().try_find(name, descriptor);
	if(m.has_value()) {
		return m.get();
	}

	/*    Otherwise, if C has a superclass, step 2 of method resolution is
	      recursively invoked on the direct superclass of C. */
	if(c.has_super()) {
		return try_method_resolution_step_2(c.super(), name, descriptor);
	}

	return optional<method&>{};
}

/* symbolic reference from D to a method in a class C is already resolved */
template<basic_range Name, basic_range Descriptor>
[[nodiscard]] expected<method&, reference>
try_resolve_method(_class& d, _class& c, Name&& name, Descriptor&& descriptor) {
	/* 1. If C is an interface, method resolution throws an
	      IncompatibleClassChangeError. */
	if(c.is_interface()) {
		expected<reference, reference> possible_icce
			= try_create_incompatible_class_change_error();
		return unexpected{ move(possible_icce.get()) };
	}

	/* 2. Otherwise, method resolution attempts to locate the referenced method
	      in C and its superclasses: */
	expected<optional<method&>, reference> m_or_throwable
		= try_method_resolution_step_2(c, name, descriptor);

	if(m_or_throwable.is_unexpected()) {
		return unexpected{ move(m_or_throwable.get_unexpected()) };
	}

	optional<method&> possible_m = m_or_throwable.get_expected();

	/* 3. Otherwise, method resolution attempts to locate the referenced method
	      in the superinterfaces of the specified class C: */
	/*    * If the maximally-specific superinterface methods of C for the name
	        and descriptor specified by the method reference include exactly one
	        method that does not have its ACC_ABSTRACT flag set, then this
	        method is chosen and method lookup succeeds. */
	if(!possible_m.has_value()) {
		c.for_each_maximally_specific_super_interface_instance_method(
			name, descriptor,
			[&](method& m0) {
				if(!m0.access_flags().abstract) {
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

	/*    Otherwise, if any superinterface of C declares a method with the name
	      and descriptor specified by the method reference that has neither
	      its ACC_PRIVATE flag nor its ACC_STATIC flag set, one of these is
	      arbitrarily chosen and method lookup succeeds. */
	if(!possible_m.has_value()) {
		c.for_each_super_interface([&](_class& i) {
			for(method& m0 : i.declared_methods()) {
				if(
					has_name_and_descriptor_equal_to{ name, descriptor }(m0) &&
					!m0.is_private() && !m0.is_static()
				) {
					possible_m = m0;
					return loop_action::stop;
				}
			}
			return loop_action::next;
		});
	}

	//    Otherwise, method lookup fails.
	/* If method lookup failed, method resolution throws a NoSuchMethodError. */
	if(!possible_m.has_value()) {
		expected<reference, reference> possible_nsme
			= try_create_no_such_method_error();
		return unexpected{ move(possible_nsme.get()) };
	}

	method& m = possible_m.get();

	/* Otherwise, method lookup succeeded. Access control is applied for the
	   access from D to the method which is the result of method lookup
	   (§5.4.4). */
	optional<reference> possible_error = access_control(d, m);
	if(possible_error.has_value()) {
		/* - If access control failed, method resolution fails for the same
		     reason. */
		return unexpected { move( possible_error.get() )};
	}

	/* - Otherwise, access control succeeded. Loading constraints are imposed,
	     as follows. */ // TODO

	return m;
}

inline expected<method&, reference> _class::try_resolve_method(
	class_file::constant::method_ref ref
) {
	/* To resolve an unresolved symbolic reference from D to a method in a
	   class C, the symbolic reference to C given by the method reference is
	   first resolved (§5.4.3.1) */
	expected<_class&, reference> possible_c
		= try_get_resolved_class(ref.class_index);

	_class& c = possible_c.get_expected();

	auto nat = name_and_type_constant(ref.name_and_type_index);
	auto name = utf8_constant(nat.name_index);
	auto descriptor = utf8_constant(nat.descriptor_index);
	return ::try_resolve_method(*this, c, name, descriptor);
}
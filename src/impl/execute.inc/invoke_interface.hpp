#include "decl/class.hpp"
#include "decl/object.hpp"
#include "decl/execute.hpp"
#include "decl/execution/stack.hpp"
#include "decl/execution/info.hpp"
#include "decl/lib/java/lang/incompatible_class_change_error.hpp"
#include "decl/lib/java/lang/null_pointer_exception.hpp"
#include "decl/lib/java/lang/illegal_access_error.hpp"
#include "decl/lib/java/lang/abstract_method_error.hpp"

#include <ranges.hpp>

#include <print/print.hpp>

[[nodiscard]] inline optional<reference> try_invoke_interface_resolved(
	method& resolved_interface_method
) {
	uint8 args_count = resolved_interface_method.parameters_count();
	++args_count; // this

	// copy, so object and it's lock may not be destroyed
	reference obj_ref = stack.get<reference>(stack.size() - args_count);

	/* Otherwise, if objectref is null, the invokeinterface instruction throws
	   a NullPointerException. */
	if(obj_ref.is_null()) {
		return try_create_null_pointer_exception().get();
	}

	/* Otherwise, if the class of objectref does not implement the resolved
	   interface, invokeinterface throws an IncompatibleClassChangeError. */
	if(!obj_ref.c().is_implementing(resolved_interface_method.c())) {
		return try_create_incompatible_class_change_error(
			(j::string&) create_string_from_utf8(
				ranges {
					u8"class "sv,
					obj_ref.c().name(),
					u8" does not implement resolved interface "sv,
					resolved_interface_method.c().name()
				}.concat_view()
			).object()
		).get();
		return try_create_incompatible_class_change_error().get();
	}

	/* Let C be the class of objectref. A method is selected with respect to C
	   and the resolved method (§5.4.6). This is the method to be invoked. */
	c& c = obj_ref.c();
	optional<method&> possible_selected_method
		= try_select_method(c, resolved_interface_method);
	
	/* Otherwise, if no method is selected,  */
	if(!possible_selected_method.has_value()) {
		nuint maximally_specific_count = 0;

		c.for_each_maximally_specific_super_interface_instance_method(
			resolved_interface_method.name(),
			resolved_interface_method.descriptor(),
			[&](method& m) {
				if(!m.is_abstract()) {
					++maximally_specific_count;
				}
			}
		);
		/* and there are multiple maximally-specific superinterface methods of C
		   that match the resolved method's name and descriptor and are not
		   abstract, invokeinterface throws an IncompatibleClassChangeError */
		if(maximally_specific_count > 1) {
			return try_create_incompatible_class_change_error().get();
		}
		/* and there are no maximally-specific superinterface methods of C that
		   match the resolved method's name and descriptor and are not abstract,
		   invokeinterface throws an AbstractMethodError. */
		else {
			return try_create_abstract_method_error().get();
		}
	}

	method& selected_method = possible_selected_method.get();

	/* Otherwise, if the selected method is neither public nor private,
	   invokeinterface throws an IllegalAccessError. */
	if(!selected_method.is_public() && !selected_method.is_private()) {
		return try_create_illegal_access_error().get();
	}

	/* Otherwise, if the selected method is abstract, invokeinterface throws an
	   AbstractMethodError. */
	if(selected_method.is_abstract()) {
		return try_create_abstract_method_error().get();
	}

	/* Otherwise, if the selected method is native and the code that implements
	   the method cannot be bound, invokeinterface throws an
	   UnsatisfiedLinkError. */
	// TODO

	if(selected_method.is_synchronized()) {
		obj_ref->lock();
	}
	on_scope_exit unlock_if_synchronized { [&] {
		if(selected_method.is_synchronized()) {
			obj_ref->unlock();
		}
	}};

	return try_execute(selected_method);
}

// c is resolved from d
template<basic_range Name, basic_range Desriptor>
[[nodiscard]] optional<reference> try_invoke_interface(
	c& c, Name&& name, Desriptor&& descriptor
) {
	expected<method&, reference> possible_resolved_method
		= try_resolve_interface_method(
			c, name , descriptor
		);

	if(possible_resolved_method.is_unexpected()) {
		return possible_resolved_method.move_unexpected();
	}

	method& resolved_method = possible_resolved_method.get_expected();
	return try_invoke_interface_resolved(resolved_method);
}

[[nodiscard]] inline optional<reference> try_invoke_interface(
	c& d, class_file::constant::interface_method_ref_index ref_index
) {
	namespace cc = class_file::constant;
	cc::interface_method_ref method_ref = d[ref_index];
	cc::name_and_type nat = d[method_ref.name_and_type_constant_index];

	cc::utf8 name = d[nat.name_constant_index];
	cc::utf8 desc = d[nat.descriptor_constant_index];
	
	expected<method&, reference> possible_resolved_method
	= d.try_get_resolved_interface_method(
		ref_index,
		[&](method& resolved_method) -> optional<reference> {
			/* Otherwise, if the resolved method is static, the invokeinterface
			instruction throws an IncompatibleClassChangeError. */
			if(resolved_method.is_static()) {
				return move(try_create_incompatible_class_change_error().get());
			}
			return {};
		}
	);

	/* During resolution of the symbolic reference to the interface method, any
	   of the exceptions pertaining to interface method resolution (§5.4.3.4)
	   can be thrown. */
	if(possible_resolved_method.is_unexpected()) {
		return possible_resolved_method.move_unexpected();
	}

	method& resolved_method = possible_resolved_method.get_expected();

	return try_invoke_interface_resolved(resolved_method);
}
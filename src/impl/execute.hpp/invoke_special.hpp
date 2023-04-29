#include "decl/execute.hpp"
#include "decl/execution/info.hpp"
#include "decl/execution/stack.hpp"
#include "decl/class.hpp"
#include "decl/method.hpp"
#include "decl/lib/java/lang/object.hpp"
#include "decl/lib/java/lang/incompatible_class_change_error.hpp"
#include "decl/lib/java/lang/null_pointer_exception.hpp"
#include "decl/lib/java/lang/no_such_method_error.hpp"
#include "decl/lib/java/lang/abstract_method_error.hpp"
#include "decl/object.hpp"

#include <loop_action.hpp>
#include <overloaded.hpp>
#include <optional.hpp>

[[nodiscard]] inline optional<reference> try_invoke_special_selected(
	method& selected_method
) {
	// copy, so object and it's lock may not be destroyed
	reference obj_ref = stack.get<reference>(
		stack.size() - selected_method.parameters_stack_size()
	);

	/* Otherwise, if objectref is null, the invokespecial instruction throws a
	   NullPointerException. */
	if(obj_ref.is_null()) {
		return try_create_null_pointer_exception().get();
	}

	/* Otherwise, if step 1, step 2, or step 3 of the lookup procedure */
	if(!selected_method._class().is_interface()) {
		/* selects an abstract method, invokespecial throws an
		   AbstractMethodError. */
		if(selected_method.is_abstract()) {
			return try_create_abstract_method_error().get();
		}
		/* selects a native method and the code that implements the method
		   cannot be bound, invokespecial throws an UnsatisfiedLinkError.*/
		// TODO
	}

	/* If the method is synchronized, the monitor associated with objectref is
	   entered or reentered as if by execution of a monitorenter instruction
	   (§monitorenter) in the current thread. */
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

[[nodiscard]] inline optional<reference> try_invoke_special_resolved(
	_class& current_c, method& resolved_method
) {

	_class& referenced_class = resolved_method._class();

	optional<instance_method&> possible_selected_method
		= select_method_for_invoke_special(
			current_c, referenced_class, resolved_method
		);

	/* Otherwise, if step 4 of the lookup procedure determines there are */
	if(possible_selected_method.has_no_value()) {
		nuint maximally_specific_count = 0;
		/* multiple maximally-specific superinterface methods of C that match
		   the resolved method's name and descriptor and are not abstract,
		   invokespecial throws an IncompatibleClassChangeError */
		current_c.for_each_maximally_specific_super_interface_instance_method(
			resolved_method.name(), resolved_method.descriptor(),
			[&](method& m) {
				if(!m.is_abstract()) {
					++maximally_specific_count;
				}
			}
		);
		/* and there are multiple maximally-specific superinterface methods of C
		   that match the resolved method's name and descriptor and are not
		   abstract, invokevirtual throws an IncompatibleClassChangeError */
		if(maximally_specific_count > 1) {
			return try_create_incompatible_class_change_error().get();
		}
		/* and there are no maximally-specific superinterface methods of C that
		   match the resolved method's name and descriptor and are not abstract,
		   invokevirtual throws an AbstractMethodError. */
		else {
			return try_create_abstract_method_error().get();
		}
	}

	method& selected_method = possible_selected_method.get();

	return try_invoke_special_selected(selected_method);
}

[[nodiscard]] inline optional<reference> try_invoke_special(
	_class& current_c,
	class_file::constant::method_or_interface_method_ref_index ref_index
) {
	auto verifier = [&](method& m) -> optional<reference> {
		class_file::constant::utf8 referenced_class_name
			= current_c.view_method_or_interface_method_constant_index(
			ref_index,
			[&](auto index) -> class_file::constant::utf8 {
				auto reference = current_c[index];
				class_file::constant::class_index ci = reference.class_index;
				class_file::constant::_class c = current_c[ci];
				return current_c[c.name_index];
			}
		);

		/* Otherwise, if the resolved method is an instance initialization
		   method, and the class in which it is declared is not the class
		   symbolically referenced by the instruction, a NoSuchMethodError is
		   thrown. */
		if(
			m.is_instance_initialisation() &&
			!m._class().name()
				.has_equal_size_and_elements(referenced_class_name))
		{
			return try_create_no_such_method_error().get();
		}

		/* Otherwise, if the resolved method is a class (static) method, the
		   invokespecial instruction throws an IncompatibleClassChangeError. */
		if(m.is_static()) {
			return try_create_incompatible_class_change_error().get();
		}

		return {};
	};

	expected<method&, reference> possible_resolved_method
	= current_c.view_method_or_interface_method_constant_index(
		ref_index,
		overloaded {
			[&](class_file::constant::method_ref_index ref_index) {
				return current_c.try_get_resolved_method(
					ref_index,
					verifier
				);
			},
			[&](class_file::constant::interface_method_ref_index ref_index) {
				return current_c.try_get_resolved_interface_method(
					ref_index,
					verifier
				);
			},
		}
	);

	if(possible_resolved_method.is_unexpected()) {
		return possible_resolved_method.move_unexpected();
	}

	method& resolved_method = possible_resolved_method.get_expected();

	return try_invoke_special_resolved(current_c, resolved_method);
}
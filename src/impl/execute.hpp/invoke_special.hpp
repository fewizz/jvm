#include "decl/execute.hpp"
#include "decl/execution/info.hpp"
#include "decl/execution/stack.hpp"
#include "decl/class.hpp"
#include "decl/method.hpp"
#include "decl/lib/java/lang/object.hpp"
#include "decl/lib/java/lang/incompatible_class_change_error.hpp"
#include "decl/lib/java/lang/null_pointer_exception.hpp"
#include "decl/lib/java/lang/abstract_method_error.hpp"
#include "decl/object.hpp"

#include <loop_action.hpp>
#include <overloaded.hpp>
#include <optional.hpp>

inline optional<reference> try_invoke_special(
	class_file::constant::method_or_interface_method_ref_index ref_index,
	_class& current
) {
	auto verifier = [](method& m) -> optional<reference> {
		/* Otherwise, if the resolved method is a class (static) method, the
		invokespecial instruction throws an IncompatibleClassChangeError. */
		if(m.is_static()) {
			return try_create_incompatible_class_change_error().get();
		}
		return {};
	};

	expected<method&, reference> possible_resolved_method
	= current.view_method_or_interface_method_constant_index(
		ref_index,
		overloaded {
			[&](class_file::constant::method_ref_index ref_index) {
				return current.try_get_resolved_method(
					ref_index,
					verifier
				);
			},
			[&](class_file::constant::interface_method_ref_index ref_index) {
				return current.try_get_resolved_interface_method(
					ref_index,
					verifier
				);
			},
		}
	);

	if(possible_resolved_method.is_unexpected()) {
		return move(possible_resolved_method.get_unexpected());
	}

	method& resolved_method =
		possible_resolved_method.get_expected();

	_class& referenced_class = resolved_method._class();

	optional<method&> possible_selected_method
		= select_method_for_invoke_special(
			current, referenced_class, resolved_method
		);
	if(!possible_selected_method.has_value()) {
		posix::abort();
	}

	method& selected_method = possible_selected_method.get();

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
	/* Otherwise, if step 4 of the lookup procedure determines there are */
	else {
		nuint maximally_specific_count = 0;
		/* multiple maximally-specific superinterface methods of C that match
		   the resolved method's name and descriptor and are not abstract,
		   invokespecial throws an IncompatibleClassChangeError */
		current.for_each_maximally_specific_super_interface_instance_method(
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
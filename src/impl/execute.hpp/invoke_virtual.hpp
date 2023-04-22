#include "decl/execute.hpp"
#include "decl/execution/info.hpp"
#include "decl/execution/stack.hpp"
#include "decl/class.hpp"
#include "decl/object.hpp"
#include "decl/class/resolve_method_type.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/null_pointer_exception.hpp"
#include "decl/lib/java/lang/incompatible_class_change_error.hpp"
#include "decl/lib/java/lang/abstract_method_error.hpp"

#include <class_file/constant.hpp>

[[nodiscard]] inline optional<reference>
try_invoke_virtual_resolved_non_polymorphic(
	method& resolved_method
) {
	/* If the resolved method is not signature polymorphic (§2.9.3), then the
	   invokevirtual instruction proceeds as follows. */

	uint8 args_stack_count = resolved_method.parameters_stack_size();

	// copy, so object and it's lock may not be destroyed
	reference obj_ref = stack.get<reference>(stack.size() - args_stack_count);

	/* Otherwise, if objectref is null, the invokevirtual instruction throws a
	   NullPointerException. */
	if(obj_ref.is_null()) {
		return try_create_null_pointer_exception().get();
	}

	/* Let C be the class of objectref. A method is selected with respect to C
	   and the resolved method (§5.4.6). This is the method to be invoked. */
	_class& c = obj_ref._class();
	optional<method&> possible_selected_method
		= try_select_method(c, resolved_method);

	/* Otherwise, if no method is selected, */
	if(!possible_selected_method.has_value()) {
		nuint maximally_specific_count = 0;
		c.for_each_maximally_specific_super_interface_instance_method(
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

	/* If the selected method is abstract, invokevirtual throws an
	   AbstractMethodError. */
	if(selected_method.is_abstract()) {
		return try_create_abstract_method_error().get();
	}

	/* Otherwise, if the selected method is native and the code that implements
	   the method cannot be bound, invokevirtual throws an
	   UnsatisfiedLinkError. */ // TODO

	/* If the method to be invoked is synchronized, the monitor associated with
	   objectref is entered or reentered as if by execution of a monitorenter
	   instruction (§monitorenter) in the current thread. */
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

template<basic_range Desriptor>
[[nodiscard]] inline optional<reference> try_invoke_virtual_resolved(
	_class& d, method& resolved_method, Desriptor&& desc
) {
	if(resolved_method.is_signature_polymorphic()) {
		nuint args_count_stack = 0;

		class_file::method_descriptor::reader reader{ desc.iterator() };
		reader.try_read_parameter_types_and_get_return_type_reader(
		[&]<typename ParamType>(ParamType) {
				args_count_stack += descriptor_type_stack_size<ParamType>;
			},
			[](auto) { posix::abort(); }
		);

		// reference to method handle is popped from stack
		// before calling invoke[Exact]!
		nuint args_beginning_positoin = stack.size() - args_count_stack;

		reference mh_ref; {
			nuint mh_ref_stack_position = args_beginning_positoin - 1;
			mh_ref = stack.pop_at<reference>(mh_ref_stack_position);
			--args_beginning_positoin;
		}

		if(resolved_method.name().has_equal_size_and_elements(
			c_string{ "invokeExact" }
		)) {
			return method_handle_try_invoke_exact(
				move(mh_ref),
				args_beginning_positoin
			);
		}
		else if(resolved_method.name().has_equal_size_and_elements(
			c_string{ "invoke" }
		)) {
			expected<reference, reference> possible_new_mt
				= try_resolve_method_type(d, resolved_method.descriptor());

			if(possible_new_mt.is_unexpected()) {
				return possible_new_mt.move_unexpected();
			}

			reference new_mt = possible_new_mt.move_expected();

			return method_handle_try_invoke(
				move(mh_ref),
				move(new_mt),
				args_beginning_positoin
			);
		}

		return {};
	}

	return try_invoke_virtual_resolved_non_polymorphic(resolved_method);
}

[[nodiscard]] inline optional<reference> try_invoke_virtual(
	_class& d, class_file::constant::method_ref_index ref_index
) {
	namespace cf = class_file;
	namespace cc = cf::constant;

	cc::method_ref method_ref = d.method_ref_constant(ref_index);
	cc::name_and_type nat =
			d.name_and_type_constant(method_ref.name_and_type_index);
	cc::utf8 desc = d.utf8_constant(nat.descriptor_index);
	cc::utf8 name = d.utf8_constant(nat.name_index);

	cc::_class c = d.class_constant(method_ref.class_index);
	cc::utf8 class_name = d.utf8_constant(c.name_index);

	expected<method&, reference> possible_resolved_method
		= d.try_get_resolved_method(
			ref_index,
			[](method& m) -> optional<reference> {
				/* Otherwise, if the resolved method is a class (static) method,
				   the invokevirtual instruction throws an
				   IncompatibleClassChangeError. */
				if(m.is_static()) {
					return try_create_incompatible_class_change_error().get();
				}

				/* Otherwise, if the resolved method is signature polymorphic
				   and declared in the java.lang.invoke.MethodHandle class, then
				   during resolution of the method type derived from the
				   descriptor in the symbolic reference to the method, any of
				   the exceptions pertaining to method type resolution
				   (§5.4.3.5) can be thrown. */
				// TODO

				/* Otherwise, if the resolved method is signature polymorphic
				   and declared in the java.lang.invoke.VarHandle class, then
				   any linking exception that may arise from invocation of the
				   invoker method handle can be thrown. No linking exceptions
				   are thrown from invocation of the valueFromMethodName,
				   accessModeType, and varHandleExactInvoker methods. */
				// TODO

				return {};
			}
		);
	
	if(possible_resolved_method.is_unexpected()) {
		return possible_resolved_method.move_unexpected();
	}

	method& resolved_method = possible_resolved_method.get_expected();
	return try_invoke_virtual_resolved(d, resolved_method, desc);
}
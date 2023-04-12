#include "decl/execute.hpp"
#include "decl/execution/info.hpp"
#include "decl/execution/stack.hpp"
#include "decl/class.hpp"
#include "decl/method.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/incompatible_class_change_error.hpp"

#include <class_file/constant.hpp>

#include <overloaded.hpp>

inline optional<reference> try_invoke_static(method& resolved_method) {
	/* A class or interface C may be initialized only as a result of: */
	// ...
	/* The execution of any one of the Java Virtual Machine instructions new,
	   getstatic, putstatic, or invokestatic that references C
	   ...
	   Upon execution of a getstatic, putstatic, or invokestatic instruction,
	   the class or interface to be initialized is the class or interface that
	   declares the resolved field or method.
	*/
	optional<reference> init_error
		= resolved_method._class().try_initialise_if_need();
	if(init_error.has_value()) {
		return move(init_error.get());
	}

	if(resolved_method.access_flags().super_or_synchronized) {
		resolved_method._class().instance()->lock();
	}
	on_scope_exit unlock_if_synchronized { [&] {
		if(resolved_method.access_flags().super_or_synchronized) {
			resolved_method._class().instance()->unlock();
		}
	}};

	return try_execute(resolved_method);
}

inline optional<reference> try_invoke_static(
	_class& d,
	class_file::constant::method_or_interface_method_ref_index ref_index
) {
	auto verifier = [](method& resolved_method) -> optional<reference> {
		/* Otherwise, if the resolved method is an instance method, the
		   invokestatic instruction throws an IncompatibleClassChangeError. */
		if(resolved_method.is_instance_initialisation()) {
			expected<reference, reference> possible_icce
				= try_create_incompatible_class_change_error();

			return move(
				possible_icce.is_unexpected() ?
				possible_icce.get_expected() :
				possible_icce.get_unexpected()
			);
		}

		/* On successful resolution of the method, the class or interface that
		   declared the resolved method is initialized (§5.5) if that class or
		   interface has not already been initialized. */
		_class& c = resolved_method._class();
		optional<reference> possible_throwable
			= c.try_initialise_if_need();

		if(possible_throwable.has_value()) {
			return move(possible_throwable.get());
		}

		return {};
	};

	/* The run-time constant pool entry at the index must be a symbolic
	   reference to a method or an interface method (§5.1), which gives the name
	   and descriptor (§4.3.3) of the method or interface method as well as a
	   symbolic reference to the class or interface in which the method or
	   interface method is to be found. The named method is resolved
	   (§5.4.3.3, §5.4.3.4). */
	expected<method&, reference> possible_resolved_method
	= d.view_method_or_interface_method_constant_index(
		ref_index,
		overloaded {
			[&](class_file::constant::method_ref_index ref_index) {
				return d.try_get_resolved_method(
					ref_index,
					verifier
				);
			},
			[&](class_file::constant::interface_method_ref_index ref_index) {
				return d.try_get_resolved_interface_method(
					ref_index,
					verifier
				);
			},
		}
	);

	/* During resolution of the symbolic reference to the method, any of the
	   exceptions pertaining to method resolution (§5.4.3.3) can be thrown. */
	if(possible_resolved_method.is_unexpected()) {
		return move(possible_resolved_method.get_unexpected());
	}

	method& resolved_method = possible_resolved_method.get_expected();

	return try_invoke_static(resolved_method);
}
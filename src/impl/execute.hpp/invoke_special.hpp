#include "decl/execute.hpp"
#include "decl/execution/info.hpp"
#include "decl/execution/stack.hpp"
#include "decl/class.hpp"
#include "decl/method.hpp"
#include "decl/lib/java/lang/object.hpp"
#include "decl/object.hpp"

#include <loop_action.hpp>

inline optional<reference> try_invoke_special(
	class_file::constant::method_ref_index ref_index, _class& current
) {
	namespace cc = class_file::constant;

	cc::method_ref method_ref = current.method_ref_constant(ref_index);
	cc::name_and_type nat {
		current.name_and_type_constant(method_ref.name_and_type_index)
	};
	
	cc::utf8 method_desc = current.utf8_constant(nat.descriptor_index);

	if(info) {
		tabs();
		print::out("invoke_special ");
		cc::_class _c = current.class_constant(method_ref.class_index);
		cc::utf8 class_name = current.utf8_constant(_c.name_index);
		cc::utf8 method_name = current.utf8_constant(nat.name_index);
		print::out(class_name, ".", method_name, method_desc, "\n");
	}

	expected<method&, reference> possible_resolved_method
		= current.try_get_resolved_method(ref_index);

	if(possible_resolved_method.is_unexpected()) {
		return move(possible_resolved_method.get_unexpected());
	}

	method& resolved_method = possible_resolved_method.get_expected();

	expected<_class&, reference> possible_referenced_class
		= current.try_get_resolved_class(method_ref.class_index);
	
	if(possible_referenced_class.is_unexpected()) {
		return move(possible_referenced_class.get_unexpected());
	}

	_class& referenced_class = possible_referenced_class.get_expected();

	method& m = select_method_for_invoke_special(
		current, referenced_class, resolved_method
	);

	// copy, so object and it's lock may not be destroyed
	reference obj_ref = stack.get<reference>(
		stack.size() - m.parameters_stack_size()
	);

	if(m.access_flags().super_or_synchronized) {
		obj_ref->lock();
	}
	on_scope_exit unlock_if_synchronized { [&] {
		if(m.access_flags().super_or_synchronized) obj_ref->unlock();
	}};

	return try_execute(m);
}
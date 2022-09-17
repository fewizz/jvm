#include "decl/execute.hpp"
#include "decl/execution/info.hpp"
#include "decl/execution/stack.hpp"
#include "decl/class.hpp"
#include "decl/method.hpp"
#include "decl/lib/java/lang/object.hpp"

#include <loop_action.hpp>

template<basic_range StackType>
inline void invoke_special(
	class_file::constant::method_ref_index ref_index,
	_class& current, StackType& stack
) {
	namespace cc = class_file::constant;

	cc::method_ref method_ref = current.method_ref_constant(ref_index);
	cc::name_and_type nat {
		current.name_and_type_constant(method_ref.name_and_type_index)
	};
	
	cc::utf8 method_desc = current.utf8_constant(nat.descriptor_index);

	if(info) {
		tabs(); print("invoke_special ");
		cc::_class _c = current.class_constant(method_ref.class_index);
		cc::utf8 class_name = current.utf8_constant(_c.name_index);
		print(class_name);
		print(".");
		cc::utf8 method_name = current.utf8_constant(nat.name_index);
		print(method_name);
		print(method_desc);
		print("\n");
	}

	method& resolved_method = current.get_resolved_method(ref_index);
	_class& referenced_class
		= current.get_resolved_class(method_ref.class_index);

	method& m = select_method_for_invoke_special(
		current, referenced_class, resolved_method
	);

	uint8 args_count = resolved_method.parameters_count();
	++args_count; // this
	optional<stack_entry> result = execute(
		m,
		arguments_span {
			&*stack.iterator() + stack.size() - args_count, args_count
		}
	);

	stack.pop_back(args_count);

	result.if_has_value([&](stack_entry& value) {
		stack.emplace_back(move(value));
	});

}
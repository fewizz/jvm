#include "decl/execute.hpp"
#include "decl/execution/info.hpp"
#include "decl/execution/stack.hpp"
#include "decl/class.hpp"
#include "decl/method.hpp"
#include "decl/lib/java/lang/object.hpp"

#include <loop_action.hpp>

inline void invoke_special(
	class_file::constant::method_ref_index ref_index,
	_class& current, stack& stack
) {
	namespace cc = class_file::constant;

	cc::method_ref method_ref = current.method_ref_constant(ref_index);
	cc::name_and_type nat {
		current.name_and_type_constant(method_ref.name_and_type_index)
	};
	
	cc::utf8 method_desc = current.utf8_constant(nat.descriptor_index);

	if(info) {
		tabs(); fputs("invoke_special ", stderr);
		cc::_class _c = current.class_constant(method_ref.class_index);
		cc::utf8 class_name = current.utf8_constant(_c.name_index);
		fwrite(class_name.elements_ptr(), 1, class_name.size(), stderr);
		fputc('.', stderr);
		cc::utf8 method_name = current.utf8_constant(nat.name_index);
		fwrite(method_name.elements_ptr(), 1, method_name.size(), stderr);
		fwrite(method_desc.elements_ptr(), 1, method_desc.size(), stderr);
		fputc('\n', stderr);
	}

	method& resolved_method = current.get_resolved_method(ref_index);
	_class& referenced_class = current.get_class(method_ref.class_index);

	method& m = select_method_for_invoke_special(
		current, referenced_class, resolved_method
	);

	uint8 args_count = resolved_method.parameters_count();
	++args_count; // this
	optional<stack_entry> result = execute(
		m,
		arguments_span {
			stack.iterator() + stack.size() - args_count, args_count
		}
	);

	stack.pop_back(args_count);

	result.if_has_value([&](stack_entry& value) {
		stack.emplace_back(move(value));
	});

}
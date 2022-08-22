#include "decl/class.hpp"
#include "decl/object.hpp"
#include "decl/execute.hpp"
#include "decl/execution/stack.hpp"
#include "decl/execution/info.hpp"

#include "./select_method.hpp"

inline void invoke_interface(
	class_file::constant::interface_method_ref_index ref_index,
	_class& c, stack& stack
) {

	namespace cc = class_file::constant;
	cc::interface_method_ref method_ref_info
		= c.interface_method_ref_constant(ref_index);
	cc::name_and_type name_and_type_info
		= c.name_and_type_constant(method_ref_info.name_and_type_index);

	auto name = c.utf8_constant(name_and_type_info.name_index);
	auto desc = c.utf8_constant(name_and_type_info.descriptor_index);
	if(info) {
		cc::_class class_info
			= c.class_constant(method_ref_info.interface_index);
		auto class_name = c.utf8_constant(class_info.name_index);
		tabs(); fputs("invoke_interface ", stderr);
		fwrite(class_name.elements_ptr(), 1, class_name.size(), stderr);
		fputc('.', stderr);
		fwrite(name.elements_ptr(), 1, name.size(), stderr);
		fwrite(desc.elements_ptr(), 1, desc.size(), stderr);
		fputc('\n', stderr);
	}

	method& resolved_method = c.resolve_interface_method(method_ref_info);

	uint8 args_count = resolved_method.parameters_count();
	++args_count; // this
	reference& objectref = stack[stack.size() - args_count].get<reference>();

	/* "Let C be the class of objectref. A method is selected with respect to C
	and the resolved method (§5.4.6). This is the method to be invoked." */
	method& m = select_method(objectref->_class(), resolved_method);
	
	optional<stack_entry> result = execute(
		m, arguments_span {
			stack.iterator() + stack.size() - args_count,
			args_count
		}
	);

	stack.pop_back(args_count);

	result.if_has_value([&](stack_entry& value) {
		stack.emplace_back(move(value));
	});
	
}
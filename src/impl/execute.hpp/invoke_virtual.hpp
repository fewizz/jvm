#include "decl/execute.hpp"
#include "decl/execution/info.hpp"
#include "decl/execution/stack.hpp"
#include "decl/class.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"

#include "./select_method.hpp"

#include <class_file/constant.hpp>

inline void invoke_virtual(
	class_file::constant::method_ref_index ref_index, _class& c, stack& stack
) {
	namespace cf = class_file;
	namespace cc = cf::constant;

	cc::method_ref method_ref { c.method_ref_constant(ref_index) };
	cc::name_and_type nat {
			c.name_and_type_constant(method_ref.name_and_type_index)
		};
	cc::utf8 desc = c.utf8_constant(nat.descriptor_index);
	cc::utf8 name = c.utf8_constant(nat.name_index);

	if(info) {
		cc::_class _c { c.class_constant(method_ref.class_index) };
		cc::utf8 class_name = c.utf8_constant(_c.name_index);
		tabs(); fputs("invoke_virtual ", stderr);
		fwrite(class_name.elements_ptr(), 1, class_name.size(), stderr);
		fputc('.', stderr);
		fwrite(name.elements_ptr(), 1, name.size(), stderr);
		fwrite(desc.elements_ptr(), 1, desc.size(), stderr);
		fputc('\n', stderr);
	}

	method& resolved_method = c.get_resolved_method(ref_index);

	optional<stack_entry> result;
	uint8 args_count = method_descriptor_parameters_count(desc);
	++args_count; // this
	reference& objectref = stack[stack.size() - args_count].get<reference>();

	if(&resolved_method._class() == method_handle_class.ptr()) {
		if(range{ name }.equals_to(c_string{ "invokeExact" })) {
			result = method_handle_invoke_exact(
				objectref, // method handle
				arguments_span {
					stack.iterator() + stack.size() - (args_count - 1),
					uint16(args_count - 1)
				}
			);
		} else {
			abort();
		}
	}
	/* "Let C be the class of objectref. A method is selected with respect to C
	and the resolved method (§5.4.6). This is the method to be invoked." */
	else {
		method& m = select_method(objectref->_class(), resolved_method);
		result = execute(
			m,
			arguments_span {
				stack.iterator() + stack.size() - args_count, args_count
			}
		);
	}

	stack.pop_back(args_count);

	result.if_has_value([&](stack_entry& value) {
		stack.emplace_back(move(value));
	});
}
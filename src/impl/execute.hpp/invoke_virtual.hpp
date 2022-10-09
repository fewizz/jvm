#include "decl/execute.hpp"
#include "decl/execution/info.hpp"
#include "decl/execution/stack.hpp"
#include "decl/class.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"

#include <class_file/constant.hpp>

inline void invoke_virtual(
	class_file::constant::method_ref_index ref_index, _class& c
) {
	namespace cf = class_file;
	namespace cc = cf::constant;

	cc::method_ref method_ref { c.method_ref_constant(ref_index) };
	cc::name_and_type nat =
			c.name_and_type_constant(method_ref.name_and_type_index);
	cc::utf8 desc = c.utf8_constant(nat.descriptor_index);
	cc::utf8 name = c.utf8_constant(nat.name_index);

	if(info) {
		cc::_class _c { c.class_constant(method_ref.class_index) };
		cc::utf8 class_name = c.utf8_constant(_c.name_index);
		tabs(); print("invoke_virtual ");
		print(class_name);
		print(".");
		print(name);
		print(desc);
		print("\n");
	}

	method& resolved_method = c.get_resolved_method(ref_index);

	optional<stack_entry> result;
	uint8 args_count = method_descriptor_parameters_count(desc);
	++args_count; // this
	reference& objectref = stack.at<reference>(stack.size() - args_count);

	if(&resolved_method._class() == method_handle_class.ptr()) {
		if(name.have_elements_equal_to(c_string{ "invokeExact" })) {
			result = method_handle_invoke_exact(
				objectref, // method handle
				parameters_count{ (uint8)(args_count - 1) }
			);
		} else {
			abort();
		}
	}
	/* "Let C be the class of objectref. A method is selected with respect to C
	and the resolved method (§5.4.6). This is the method to be invoked." */
	else {
		method& m = select_method(objectref->_class(), resolved_method);
		execute(m);
	}
}
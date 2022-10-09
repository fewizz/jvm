#include "decl/class.hpp"
#include "decl/object.hpp"
#include "decl/execute.hpp"
#include "decl/execution/stack.hpp"
#include "decl/execution/info.hpp"
#include "decl/print.hpp"

inline void invoke_interface(
	class_file::constant::interface_method_ref_index ref_index, _class& c
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
		tabs(); print("invoke_interface ");
		print(class_name);
		print(".");
		print(name);
		print(desc);
		print("\n");
	}

	method& resolved_method = c.resolve_interface_method(method_ref_info);

	uint8 args_count = resolved_method.parameters_count();
	++args_count; // this
	reference& objectref = stack.at<reference>(stack.size() - args_count);

	/* "Let C be the class of objectref. A method is selected with respect to C
	    and the resolved method (§5.4.6). This is the method to be invoked." */
	method& m = select_method(objectref->_class(), resolved_method);
	execute(m);
}
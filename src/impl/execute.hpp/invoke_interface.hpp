#include "decl/class.hpp"
#include "decl/object.hpp"
#include "decl/execute.hpp"
#include "decl/execution/stack.hpp"
#include "decl/execution/info.hpp"

#include <print/print.hpp>

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
		tabs();
		print::out("invoke_interface ", class_name, ".", name, desc, "\n");
	}

	method& resolved_method = c.resolve_interface_method(method_ref_info);

	uint8 args_count = resolved_method.parameters_count();
	++args_count; // this

	// copy, so object and it's lock may not be destroyed
	reference obj_ref = stack.get<reference>(stack.size() - args_count);

	/* "Let C be the class of objectref. A method is selected with respect to C
	    and the resolved method (§5.4.6). This is the method to be invoked." */
	method& m = select_method(obj_ref->_class(), resolved_method);

	if(m.access_flags().super_or_synchronized) {
		obj_ref->lock();
	}
	on_scope_exit unlock_if_synchronized { [&] {
		if(m.access_flags().super_or_synchronized) obj_ref->unlock();
	}};

	execute(m);
}
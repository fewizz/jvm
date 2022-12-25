#include "decl/execute.hpp"
#include "decl/execution/info.hpp"
#include "decl/execution/stack.hpp"
#include "decl/class.hpp"
#include "decl/object.hpp"
#include "decl/thrown.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/null_pointer_exception.hpp"

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

	cc::_class _c { c.class_constant(method_ref.class_index) };
	cc::utf8 class_name = c.utf8_constant(_c.name_index);

	if(info) {
		tabs(); print("invoke_virtual ");
		print(class_name);
		print(".");
		print(name);
		print(desc);
		print("\n");
	}

	if(class_name.have_elements_equal_to(
		c_string{ "java/lang/invoke/MethodHandle" }
	)) {
		nuint args_count_stack = 0;

		class_file::method_descriptor::reader reader{ desc.iterator() };
		reader.try_read_parameter_types_and_get_return_type_reader(
		[&]<typename ParamType>(ParamType) {
				if constexpr(same_as<ParamType, class_file::v>) {
					__builtin_unreachable();
				} else {
					args_count_stack += 
						same_as_any<ParamType, class_file::j, class_file::d> ?
						2 : 1;
				}
			},
			[](auto) { abort(); }
		);
		reference& mh
			= stack.get<reference>(stack.size() - args_count_stack - 1);

		if(name.have_elements_equal_to(c_string{ "invokeExact" })) {
			method_handle_invoke_exact(
				mh, // method handle
				args_count_stack
			);
		} else {
			abort();
		}
		return;
	}

	method& resolved_method = c.get_resolved_method(ref_index);

	uint8 args_stack_count = resolved_method.parameters_stack_size();

	// copy, so object and it's lock may not be destroyed
	reference obj_ref = stack.get<reference>(stack.size() - args_stack_count);

	if(obj_ref.is_null()) {
		thrown = create_null_pointer_exception();
		return;
	}

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
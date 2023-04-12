#include "decl/execute.hpp"
#include "decl/execution/info.hpp"
#include "decl/execution/stack.hpp"
#include "decl/class.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/null_pointer_exception.hpp"

#include <class_file/constant.hpp>

template<basic_range Desriptor>
[[nodiscard]] inline optional<reference> try_invoke_virtual(
	method& resolved_method, Desriptor&& desc
) {
	if(resolved_method.is_signature_polymorphic()) {
		nuint args_count_stack = 0;

		class_file::method_descriptor::reader reader{ desc.iterator() };
		reader.try_read_parameter_types_and_get_return_type_reader(
		[&]<typename ParamType>(ParamType) {
				args_count_stack += descriptor_type_stack_size<ParamType>;
			},
			[](auto) { posix::abort(); }
		);

		// reference to method handle is popped from stack
		// before calling invoke[Exact]!
		nuint args_beginning_positoin = stack.size() - args_count_stack;

		reference mh_ref; {
			nuint mh_ref_stack_position = args_beginning_positoin - 1;
			mh_ref = stack.pop_at<reference>(mh_ref_stack_position);
			--args_beginning_positoin;
		}

		if(
			resolved_method.name()
			.has_equal_size_and_elements(c_string{ "invokeExact" })
		) {
			return method_handle_invoke_exact(
				move(mh_ref), args_beginning_positoin
			);
		} else {
			posix::abort();
		}

		return {};
	}

	uint8 args_stack_count = resolved_method.parameters_stack_size();

	// copy, so object and it's lock may not be destroyed
	reference obj_ref = stack.get<reference>(stack.size() - args_stack_count);

	if(obj_ref.is_null()) {
		expected<reference, reference> possible_npe
			= try_create_null_pointer_exception();
		return { move(
			possible_npe.is_unexpected() ?
			possible_npe.get_unexpected() :
			possible_npe.get_expected()
		) };
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

	return try_execute(m);
}

[[nodiscard]] inline optional<reference> try_invoke_virtual(
	_class& d, class_file::constant::method_ref_index ref_index
) {
	namespace cf = class_file;
	namespace cc = cf::constant;

	cc::method_ref method_ref = d.method_ref_constant(ref_index);
	cc::name_and_type nat =
			d.name_and_type_constant(method_ref.name_and_type_index);
	cc::utf8 desc = d.utf8_constant(nat.descriptor_index);
	cc::utf8 name = d.utf8_constant(nat.name_index);

	cc::_class _c { d.class_constant(method_ref.class_index) };
	cc::utf8 class_name = d.utf8_constant(_c.name_index);

	expected<method&, reference> possible_resolved_method
		= d.try_get_resolved_method(ref_index);
	
	if(possible_resolved_method.is_unexpected()) {
		return move(possible_resolved_method.get_unexpected());
	}

	method& resolved_method = possible_resolved_method.get_expected();
	return try_invoke_virtual(resolved_method, desc);
}
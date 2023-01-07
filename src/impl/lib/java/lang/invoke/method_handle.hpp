#include "decl/lib/java/lang/invoke/method_handle.hpp"

#include "decl/method.hpp"
#include "decl/classes.hpp"
#include "decl/native/environment.hpp"
#include "decl/object.hpp"

#include <class_file/constant.hpp>

static void init_java_lang_invoke_method_handle() {
	method_handle_class = classes.find_or_load(
		c_string{ "java/lang/invoke/MethodHandle" }
	);

	method_handle_member_instance_field_position =
		method_handle_class->instance_field_position(
			c_string{ "member_" }, c_string{ "J" }
		);

	method_handle_kind_instance_field_position =
		method_handle_class->instance_field_position(
			c_string{ "kind_" }, c_string{ "B" }
		);
}

inline reference create_method_handle(
	auto& member, class_file::constant::method_handle::behavior_kind kind
) {
	reference ref = create_object(method_handle_class.get());
	ref->set(method_handle_member_instance_field_position, (int64) &member);
	ref->set(method_handle_kind_instance_field_position, (int8) kind);
	return ref;
}

inline reference create_method_handle_invoke_static(method& m) {
	return create_method_handle(
		m, class_file::constant::method_handle::behavior_kind::invoke_static
	);
}

inline reference create_method_handle_invoke_virtual(method& m) {
	return create_method_handle(
		m, class_file::constant::method_handle::behavior_kind::invoke_virtual
	);
}

inline reference create_method_handle_new_invoke_special(method& m) {
	return create_method_handle(
		m,
		class_file::constant::method_handle::behavior_kind::new_invoke_special
	);
}

inline reference create_method_handle_invoke_special(method& m) {
	return create_method_handle(
		m, class_file::constant::method_handle::behavior_kind::invoke_special
	);
}

inline reference create_method_handle_invoke_interface(method& m) {
	return create_method_handle(
		m, class_file::constant::method_handle::behavior_kind::invoke_interface
	);
}

inline void method_handle_invoke_exact(
	[[maybe_unused]] reference ref, nuint args_stack_count
) {
	using behavior_kind = class_file::constant::method_handle::behavior_kind;

	void* member = (void*)
		ref->get<int64>(method_handle_member_instance_field_position);
	behavior_kind kind = (behavior_kind) (uint8)
		ref->get<int32>(method_handle_kind_instance_field_position);

	switch (kind) {
		case behavior_kind::invoke_virtual: {
			method& resolved_method = * (method*) member;
			reference& objectref =
				stack.get<reference>(stack.size() - args_stack_count);
			method& m = select_method(objectref->_class(), resolved_method);
			return execute(m);
		}
		case behavior_kind::invoke_static: {
			return execute(* (method*) member);
		}
		case behavior_kind::invoke_special: {
			return execute(* (method*) member);
		}
		case behavior_kind::new_invoke_special: {
			method& init = * (method*) member;
			_class& c = init._class();
			reference ref = create_object(c);

			// shifting args to add instance reference at the beginning
			stack.insert_at(stack.size() - init.parameters_stack_size(), ref);

			// calling constructor
			execute(init);

			stack.emplace_back(move(ref));
			return;
		}
		case behavior_kind::invoke_interface: {
			return execute(* (method*) member);
		}
		default: posix::abort();
	}
}

/*inline optional<stack_entry> method_handle_invoke(
	reference ref, parameters_count count
) {
	posix::abort();
	/using behavior_kind = class_file::constant::method_handle::behavior_kind;
	void* member = (void*) (uint64)
		ref[method_handle_member_instance_field_index].get<jlong>();
	behavior_kind kind = (behavior_kind) (uint8)
		ref[method_handle_kind_instance_field_index].get<jint>();
	if(
		kind == behavior_kind::invoke_interface ||
		kind == behavior_kind::invoke_special ||
		kind == behavior_kind::invoke_static ||
		kind == behavior_kind::invoke_virtual
	) {
		//method& m = * (method*) member;
		//m.descriptor().
	}
	else {
		posix::abort();
	}
}*/
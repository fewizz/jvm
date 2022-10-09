#include "decl/lib/java/lang/invoke/method_handle.hpp"

#include "decl/method.hpp"
#include "decl/classes.hpp"
#include "decl/native/interface/environment.hpp"
#include "decl/object/reference.hpp"
#include "decl/object.hpp"
#include "decl/object/create.hpp"

#include <class_file/constant.hpp>

static void init_java_lang_invoke_method_handle() {
	method_handle_class = classes.find_or_load(
		c_string{ "java/lang/invoke/MethodHandle" }
	);

	method_handle_member_instance_field_index = {
		(uint16) method_handle_class->instance_fields().find_index_of(
			c_string{ "member_" }, c_string{ "J" }
		)
	};

	method_handle_kind_instance_field_index = {
		(uint16) method_handle_class->instance_fields().find_index_of(
			c_string{ "kind_" }, c_string{ "B" }
		)
	};
}

inline reference create_method_handle(
	auto& member, class_file::constant::method_handle::behavior_kind kind
) {
	reference ref = create_object(method_handle_class.value());
	ref->values()[method_handle_member_instance_field_index]
		= jlong{ (int64) &member };
	ref->values()[method_handle_kind_instance_field_index]
		= jbyte{ (int8) kind };
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

inline optional<stack_entry> method_handle_invoke_exact(
	[[maybe_unused]] reference ref, [[maybe_unused]] parameters_count count
) {
	abort(); // TODO
	/*using behavior_kind = class_file::constant::method_handle::behavior_kind;
	span<stack_entry> args {
		&stack.back() - count, count
	};

	void* member = (void*) (uint64)
		ref[method_handle_member_instance_field_index].get<jlong>();
	behavior_kind kind = (behavior_kind) (uint8)
		ref[method_handle_kind_instance_field_index].get<jint>();
	
	switch (kind) {
		case behavior_kind::invoke_virtual: {
			method& resolved_method = * (method*) member;
			reference& objectref = args[0].get<reference>();
			method& m = select_method(objectref->_class(), resolved_method);
			return execute(m, count);
		}
		case behavior_kind::invoke_static: {
			return execute(* (method*) member, count);
		}
		case behavior_kind::invoke_special: {
			return execute(* (method*) member, count);
		}
		case behavior_kind::new_invoke_special: {
			method& init = * (method*) member;
			_class& c = init._class();
			reference ref = create_object(c);

			// moving args to add instance ref at begining
			stack.emplace_back(jint{});

			for(int i = 0; i < count; ++i) {
				stack[stack.size() - i - 1] = move(stack[stack.size() - i - 2]);
			}
			// adding instance ref
			stack[stack.size() - count - 1] = ref;
			++count;

			execute(init, count);
			return stack_entry{ ref };
		}
		case behavior_kind::invoke_interface: {
			return execute(* (method*) member, count);
		}
		default: abort();
	}*/
}

/*inline optional<stack_entry> method_handle_invoke(
	reference ref, parameters_count count
) {
	abort();
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
		abort();
	}
}*/
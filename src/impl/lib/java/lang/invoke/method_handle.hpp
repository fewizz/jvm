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
		method_handle_class->instance_fields().find_index_of(
			c_string{ "member_" }, c_string{ "J" }
		)
	};

	method_handle_kind_instance_field_index = {
		method_handle_class->instance_fields().find_index_of(
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

inline reference create_method_handle_invoke_special(method& m) {
	return create_method_handle(
		m, class_file::constant::method_handle::behavior_kind::invoke_special
	);
}

inline optional<stack_entry> method_handle_invoke_exact(
	reference ref, arguments_span args
) {
	using behavior_kind = class_file::constant::method_handle::behavior_kind;
	void* member = (void*) (uint64)
		ref->values() [method_handle_member_instance_field_index].get<jlong>();
	behavior_kind kind = (behavior_kind) (uint8)
		ref->values() [method_handle_kind_instance_field_index].get<jint>();
	
	switch (kind) {
		case behavior_kind::invoke_virtual: {
			method& resolved_method = * (method*) member;
			reference& objectref = args[0].get<reference>();
			method& m = select_method(objectref->_class(), resolved_method);
			return execute(m, args);
		}
		case behavior_kind::invoke_static: {
			return execute(
				* (method*) member,
				args
			);
		}
		case behavior_kind::invoke_special: { // TODO exclude <init>
			return execute(
				* (method*) member,
				args
			);
		}
		default: abort();
	}
}
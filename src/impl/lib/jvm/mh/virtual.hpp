#include "decl/lib/jvm/mh/virtual.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"
#include "decl/lib/jvm/mh/class_member.hpp"
#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/invoke/wrong_method_type_exception.hpp"
#include "decl/execute.hpp"

static void init_jvm_mh_virtual() {
	mh_virtual_class = classes.load_class_by_bootstrap_class_loader(
		c_string{"jvm/mh/Virtual"}
	);

	mh_virtual_constructor = mh_virtual_class->declared_instance_methods().find(
		c_string{"<init>"},
		c_string{"(Ljava/lang/invoke/MethodType;Ljava/lang/Class;S)V"}
	);

	mh_virtual_class->declared_instance_methods().find(
		c_string{"invokeExactPtr"}, c_string{"()V"}
	).native_function(
		(void*)+[](reference mh, [[maybe_unused]] nuint args_beginning)
		-> optional<reference>
		{
			reference& c_ref
				= mh->get<reference>(mh_class_member_class_position);

			c& c = class_from_class_instance(c_ref);

			declared_instance_method_index resolved_method_index {
				mh->get<uint16>(mh_class_member_index_position)
			};

			method& resolved_method = c[resolved_method_index];

			return try_invoke_virtual_resolved_non_polymorphic(resolved_method);
		}
	);
}
#include "decl/lib/jvm/mh/special.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"
#include "decl/lib/jvm/mh/class_member.hpp"
#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/invoke/wrong_method_type_exception.hpp"
#include "decl/execute.hpp"

static void init_jvm_mh_special() {
	mh_special_class = classes.load_class_by_bootstrap_class_loader(
		c_string{"jvm/mh/Special"}
	);

	mh_special_constructor = mh_special_class->declared_instance_methods().find(
		c_string{"<init>"},
		c_string{"(Ljava/lang/invoke/MethodType;Ljava/lang/Class;S)V"}
	);

	mh_special_class->declared_instance_methods().find(
		c_string{"invokeExactPtr"}, c_string{"()V"}
	).native_function(
		(void*)+[](
			reference mh,
			[[maybe_unused]] nuint args_beginning
		) -> optional<reference> {
			declared_instance_method_index method_index {
				mh->get<uint16>(mh_class_member_index_position)
			};

			reference& c_ref
				= mh->get<reference>(mh_class_member_class_position);
			c& c = class_from_class_instance(c_ref);

			method& selected_method = c[method_index];

			return try_invoke_special_selected(selected_method);
		}
	);
}
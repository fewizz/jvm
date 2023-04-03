#include "decl/lib/jvm/mh/constructor.hpp"

#include "decl/classes.hpp"
#include "decl/lib/jvm/mh/class_member.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/invoke/wrong_method_type_exception.hpp"
#include "decl/execute.hpp"
#include "decl/thrown.hpp"

static void init_jvm_mh_constructor() {
	mh_constructor_class = classes.load_class_by_bootstrap_class_loader(
		c_string{"jvm/mh/Constructor"}
	);

	mh_constructor_constructor =
		mh_constructor_class->declared_instance_methods().find(
			c_string{"<init>"},
			c_string{"(Ljava/lang/invoke/MethodType;Ljava/lang/Class;S)V"}
		);

	mh_constructor_class->declared_instance_methods().find(
		c_string{"invokeExactPtr"}, c_string{"()V"}
	).native_function(
		(void*)+[](
			reference mh,
			[[maybe_unused]] nuint args_beginning
		) -> void {
			declared_instance_method_index method_index {
				mh->get<uint16>(mh_class_member_index_position)
			};
			reference& refc_ref
				= mh->get<reference>(mh_class_member_class_position);
			_class& refc = class_from_class_instance(refc_ref);

			method& constructor = refc[method_index];
			reference result = create_object(refc);
			stack.insert_at(args_beginning, result);
			execute(constructor);
			stack.emplace_back(result);
		}
	);
}
#include "decl/lib/jvm/mh/constructor.hpp"

#include "decl/classes.hpp"
#include "decl/lib/jvm/mh/class_member.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/invoke/wrong_method_type_exception.hpp"
#include "decl/execute.hpp"
#include "decl/thrown.hpp"

static void init_jvm_mh_constructor() {
	mh_constructor_class = classes.find_or_load(c_string{"jvm/mh/Constructor"});

	mh_constructor_constructor =
		mh_constructor_class->declared_instance_methods().find(
			c_string{"<init>"},
			c_string{"(Ljava/lang/invoke/MethodType;Ljava/lang/Class;I)V"}
		);

	mh_constructor_class->declared_instance_methods().find(
		c_string{"invokeExactPtr"}, c_string{"()V"}
	).native_function(
		(void*)+[](
			reference mh,
			[[maybe_unused]] nuint args_beginning
		) -> void {
			uint32 index = mh->get<int32>(mh_class_member_index_position);
			reference& refc_ref
				= mh->get<reference>(mh_class_member_class_position);
			_class& refc = class_from_class_instance(refc_ref);

			method& constructor = refc.declared_instance_methods()[index];
			reference result = create_object(refc);
			stack.insert_at(args_beginning, result);
			execute(constructor);
			stack.emplace_back(result);
		}
	);
}
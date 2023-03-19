#include "decl/lib/jvm/mh/virtual.hpp"

#include "decl/classes.hpp"
#include "decl/lib/jvm/mh/class_member.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/invoke/wrong_method_type_exception.hpp"
#include "decl/execute.hpp"
#include "decl/thrown.hpp"

static void init_jvm_mh_virtual() {
	mh_virtual_class = classes.find_or_load(c_string{"jvm/mh/Virtual"});

	mh_virtual_constructor = mh_virtual_class->declared_instance_methods()
		.find(c_string{"<init>"}, c_string{"(Ljava/lang/Class;I)V"});

	mh_virtual_class->declared_static_methods().find(
		c_string{"functionPtr"}, c_string{"()J"}
	).native_function(
		(void*)+[]() -> uint64 {
			return (uint64)+[](reference mh, nuint args_beginning) -> void {
				uint32 index = mh->get<int32>(mh_class_member_index_position);
				reference& refc_ref
					= mh->get<reference>(mh_class_member_class_position);

				_class& refc = class_from_class_instance(refc_ref);

				reference& obj_ref = stack.get<reference>(args_beginning);

				bool valid =
					obj_ref->_class().is(refc) ||
					obj_ref->_class().is_sub_of(refc);

				if(!valid) {
					thrown = create_wrong_method_type_exception();
					return;
				}

				execute(obj_ref._class().instance_methods()[index]);
			};
		}
	);
}
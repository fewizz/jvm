#include "decl/lib/jvm/mh/class_member.hpp"

#include "classes.hpp"

static void init_jvm_mh_class_member() {
	mh_class_member_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"jvm/mh/ClassMember" }
	);

	mh_class_member_class_position
		= mh_class_member_class->instance_field_position(
			c_string{ u8"class_" }, c_string{ u8"Ljava/lang/Class;" }
		);

	mh_class_member_index_position
		= mh_class_member_class->instance_field_position(
			c_string{ u8"memberIndex_" }, c_string{ u8"S" }
		);
}
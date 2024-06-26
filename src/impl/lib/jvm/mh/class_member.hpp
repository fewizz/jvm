#include "decl/lib/jvm/mh/class_member.hpp"

#include "classes.hpp"

static void init_jvm_mh_class_member() {
	jvm::class_member::c = classes.load_class_by_bootstrap_class_loader(
		u8"jvm/mh/ClassMember"sv
	);

	jvm::class_member::member_class_position
		= jvm::class_member::c->instance_field_position(
			u8"class_"sv, u8"Ljava/lang/Class;"sv
		);

	jvm::class_member::member_index_position
		= jvm::class_member::c->instance_field_position(
			u8"memberIndex_"sv, u8"S"sv
		);
}
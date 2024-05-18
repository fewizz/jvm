#include "decl/lib/jvm/mh/constructor.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"
#include "decl/lib/jvm/mh/class_member.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/execute.hpp"

static void init_jvm_mh_constructor() {
	mh_constructor_class = classes.load_class_by_bootstrap_class_loader(
		u8"jvm/mh/Constructor"sv
	);

	mh_constructor_constructor =
		mh_constructor_class->declared_instance_methods().find(
			u8"<init>"sv,
			u8"("
				"Ljava/lang/invoke/MethodType;"
				"Z"
				"Ljava/lang/Class;"
				"S"
			")V"sv
		);

	mh_constructor_class->declared_instance_methods().find(
		u8"invokeExactPtr"sv, u8"()V"sv
	).native_function(
		(void*)+[](
			j::method_handle& ths0
		) -> optional<reference> {
			jvm::class_member& ths = (jvm::class_member&) ths0;

			instance_method& constructor
				= ths.member<declared_instance_method_index>();

			optional<reference> optional_throwable
				= constructor.c().try_initialise_if_need();//TODO
			if(optional_throwable.has_value()) {
				return move(optional_throwable.get());
			}

			expected<reference, reference> possible_result
				= try_create_object(constructor.c());

			if(possible_result.is_unexpected()) {
				return possible_result.move_unexpected();
			}

			reference result = possible_result.move_expected();

			nuint args_beginning =
				stack.size() - ths.method_type().compute_args_stack_size();

			stack.insert_at(args_beginning, result);

			optional<reference> possible_throwable
				= try_invoke_special_selected(constructor);

			if(possible_throwable.has_value()) {
				return possible_throwable.move();
			}

			stack.emplace_back(move(result));
			return {};
		}
	);
}
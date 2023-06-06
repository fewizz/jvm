#include "decl/lib/jvm/mh/constructor.hpp"

#include "decl/classes.hpp"
#include "decl/lib/jvm/mh/class_member.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/class.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/invoke/wrong_method_type_exception.hpp"
#include "decl/execute.hpp"

static void init_jvm_mh_constructor() {
	mh_constructor_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"jvm/mh/Constructor" }
	);

	mh_constructor_constructor =
		mh_constructor_class->declared_instance_methods().find(
			c_string{ u8"<init>" },
			c_string{ u8"(Ljava/lang/invoke/MethodType;Ljava/lang/Class;S)V" }
		);

	mh_constructor_class->declared_instance_methods().find(
		c_string{ u8"invokeExactPtr" }, c_string{ u8"()V" }
	).native_function(
		(void*)+[](
			jvm::class_member& mh
		) -> optional<reference> {

			instance_method& constructor
				= mh.member<declared_instance_method_index>();

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
				stack.size() - mh.method_type().compute_args_stack_size();

			stack.insert_at(args_beginning, result);

			optional<reference> possible_throwable
				= try_invoke_special_selected(constructor);

			if(possible_throwable.has_value()) {
				return possible_throwable.move();
			}

			stack.emplace_back(result);
			return {};
		}
	);
}
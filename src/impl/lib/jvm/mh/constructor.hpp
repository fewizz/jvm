#include "decl/lib/jvm/mh/constructor.hpp"

#include "decl/classes.hpp"
#include "decl/lib/jvm/mh/class_member.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/class.hpp"
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
			reference mh,
			nuint args_beginning
		) -> optional<reference> {
			reference& c_ref
				= mh->get<reference>(mh_class_member_class_position);
			c& c = class_from_class_instance(c_ref);

			optional<reference> optional_throwable
				= c.try_initialise_if_need();//TODO
			if(optional_throwable.has_value()) {
				return move(optional_throwable.get());
			}

			expected<reference, reference> possible_result
				= try_create_object(c);

			if(possible_result.is_unexpected()) {
				return possible_result.move_unexpected();
			}

			reference result = possible_result.move_expected();

			stack.insert_at(args_beginning, result);

			declared_instance_method_index method_index {
				mh->get<uint16>(mh_class_member_index_position)
			};
			instance_method& constructor = c[method_index];

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
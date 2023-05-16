#include "decl/lib/java/lang/invoke/method_handle.hpp"

#include "decl/classes.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"
#include "decl/lib/jvm/mh/invoke_adapter.hpp"

inline void init_java_lang_invoke_method_handle() {
	method_handle_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/invoke/MethodHandle" }
	);

	method_handle_invoke_exact_ptr_index
		= method_handle_class->instance_methods().find_index_of(
			c_string{ u8"invokeExactPtr" }, c_string{ u8"()V" }
		);
	
	method_handle_method_type_field_position
		= method_handle_class->instance_field_position(
			c_string{ u8"methodType_" },
			c_string{ u8"Ljava/lang/invoke/MethodType;" }
		);

	method_handle_class->declared_instance_methods().find(
		c_string{ u8"invokePtr" }, c_string{ u8"()V" }
	).native_function(
		(void*)+[](
			reference mh_ref, reference new_mt, nuint args_beginning
		) -> optional<reference> {
			reference& mt = mh_ref->get<reference>(
				method_handle_method_type_field_position
			);
			object_of<jl::i::method_handle>& mh =
				(object_of<jl::i::method_handle>&) mh_ref.object();

			return mh::try_invoke_checked(
				mh,
				(object_of<jl::i::method_type>&) new_mt.object(),
				(object_of<jl::i::method_type>&)mt,
				args_beginning
			);
		}
	);

	method_handle_class->declared_instance_methods().find(
		c_string{ u8"asType" },
		c_string {
			u8"(Ljava/lang/invoke/MethodType;)"
			  "Ljava/lang/invoke/MethodHandle;"
		}
	).native_function(
		(void*)+[](native_environment*, object* ths, object* mt) -> object* {
			expected<reference, reference> possible_adapter = try_create_object(
				mh_invoke_adapter_constructor.get(),
				reference{*mt}  /* new MethodType */,
				reference{*ths} /* original MethodHandle */
			);
			if(possible_adapter.is_unexpected()) {
				thrown_in_native = possible_adapter.move_unexpected();
			}
			reference adapter = possible_adapter.move();
			return & adapter.unsafe_release_without_destroing();
		}
	);
}
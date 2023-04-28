#include "decl/lib/java/lang/invoke/method_handle.hpp"

#include "decl/method.hpp"
#include "decl/classes.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"
#include "decl/object.hpp"
#include "decl/lib/jvm/mh/invoke_adapter.hpp"

#include <class_file/constant.hpp>

inline void init_java_lang_invoke_method_handle() {
	method_handle_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/invoke/MethodHandle" }
	);

	method_handle_invoke_exact_ptr_index
		= method_handle_class->instance_methods().find_index_of(
			c_string{"invokeExactPtr"}, c_string{"()V"}
		);
	
	method_handle_method_type_field_position
		= method_handle_class->instance_field_position(
			c_string{"methodType_"}, c_string{"Ljava/lang/invoke/MethodType;"}
		);

	method_handle_class->declared_instance_methods().find(
		c_string{"asType"},
		c_string {
			"(Ljava/lang/invoke/MethodType;)"
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

[[nodiscard]] inline optional<reference>
method_handle_try_invoke_exact(reference mh_ref, nuint args_beginning) {
	method& m = mh_ref->_class().instance_methods()
		[method_handle_invoke_exact_ptr_index];

	void* ptr0 = m.native_function();
	using f = optional<reference>(*)(reference mh_ref, nuint args_beginning);
	return ((f)ptr0)(move(mh_ref), args_beginning);
}
#include "decl/lib/java/lang/invoke/method_handle.hpp"

#include "decl/classes.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"
#include "decl/lib/jvm/mh/invoke_adapter.hpp"

inline void init_java_lang_invoke_method_handle() {
	j::method_handle::c = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/invoke/MethodHandle" }
	);

	j::method_handle::invoke_exact_ptr_index
		= j::method_handle::c->instance_methods().find_index_of(
			c_string{ u8"invokeExactPtr" }, c_string{ u8"()V" }
		);

	j::method_handle::invoke_ptr_index
		= j::method_handle::c->instance_methods().find_index_of(
			c_string{ u8"invokePtr" }, c_string{ u8"()V" }
		);

	j::method_handle::is_varargs_instance_method
		= j::method_handle::c->instance_methods().find(
			c_string{ u8"isVarargsCollector" }, c_string{ u8"()Z" }
		);

	j::method_handle::method_type_field_position
		= j::method_handle::c->instance_field_position(
			c_string{ u8"methodType_" },
			c_string{ u8"Ljava/lang/invoke/MethodType;" }
		);

	j::method_handle::c->declared_instance_methods().find(
		c_string{ u8"invokePtr" }, c_string{ u8"()V" }
	).native_function(
		(void*)+[](
			j::method_type& t0_mt,
			j::method_handle& t1_mh
		) -> optional<reference> {
			return mh::try_invoke_unchecked(
				t0_mt,
				t1_mh
			);
		}
	);

	j::method_handle::c->declared_instance_methods().find(
		c_string{ u8"asType" },
		c_string {
			u8"(Ljava/lang/invoke/MethodType;)"
			  "Ljava/lang/invoke/MethodHandle;"
		}
	).native_function(
		(void*)+[](native_environment*, object* ths, object* mt)
		-> object*
		{
			expected<reference, reference> possible_adapter = try_create_object(
				jvm::invoke_adapter::constructor.get(),
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
j::method_handle::try_invoke_with_arguments(object& params_array) {
	nuint args_count = array_length(params_array);

	for(reference ref : array_as_span<reference>(params_array)) {
		stack.emplace_back(move(ref));
	}

	j::method_type& t1_mt = method_type();
	auto t1_params = t1_mt.parameter_types_view();
	::c& t1_ret = t1_mt.return_type();
	::c& t0_ret = object_class.get();

	if(!is_varargs() && args_count != t1_mt.parameter_types_count()) {
		posix::abort();
	}

	// TODO
	return view_on_stack<char>(args_count)([&](span<char> s)
	-> optional<reference> {
		auto t0_params = s.transform_view([](auto) -> ::c& {
			return object_class.get();
		});

		// TODO check varargs
		if(
			!is_varargs() &&
			!mh::is_convertible(t0_params, t0_ret, t1_params, t1_ret)
		) {
			posix::abort();
		}

		j::method_handle& t1_mh = *this;
		return mh::try_invoke_unchecked(t0_params, t0_ret, t1_mh);
	}
	);
}
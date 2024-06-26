#include "decl/lib/java/lang/invoke/method_handle.hpp"

#include "decl/classes.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"
#include "decl/lib/jvm/mh/invoke_adapter.hpp"
#include "decl/lib/java/lang/invoke/wrong_method_type_exception.hpp"

inline void init_java_lang_invoke_method_handle() {
	j::method_handle::c = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/invoke/MethodHandle"sv
	);

	j::method_handle::invoke_exact_ptr_index
		= j::method_handle::c->instance_methods().find_index_of(
			u8"invokeExactPtr"sv, u8"()V"sv
		);

	j::method_handle::invoke_ptr_index
		= j::method_handle::c->instance_methods().find_index_of(
			u8"invokePtr"sv, u8"()V"sv
		);

	j::method_handle::is_varargs_field_position
		= j::method_handle::c->instance_field_position(
			u8"isVarargs_"sv, u8"Z"sv
		);

	j::method_handle::method_type_field_position
		= j::method_handle::c->instance_field_position(
			u8"methodType_"sv,
			u8"Ljava/lang/invoke/MethodType;"sv
		);

	j::method_handle::c->declared_instance_methods().find(
		u8"invokePtr"sv, u8"()V"sv
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
		u8"asType"sv,
		u8"(Ljava/lang/invoke/MethodType;)"
		   "Ljava/lang/invoke/MethodHandle;"sv
	).native_function(
		(void*)+[](
			native_environment*, j::method_handle* ths, j::method_type* mt
		)
		-> object*
		{
			expected<reference, reference> possible_adapter = try_create_object(
				jvm::invoke_adapter::constructor.get(),
				*mt  /* new MethodType */,
				ths->is_varargs(),
				*ths /* original MethodHandle */
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

	// TODO
	return view_on_stack<char>(args_count)([&](span<char> s)
	-> optional<reference> {
		auto t0_params = s.transform_view([](auto&) -> ::c& {
			return object_class.get();
		});

		if(
			!mh::check(t0_params, t0_ret, t1_params, t1_ret, is_varargs())
		) {
			return try_create_wrong_method_type_exception().move();
		}

		j::method_handle& t1_mh = *this;
		return mh::try_invoke_unchecked(t0_params, t0_ret, t1_mh);
	});
}
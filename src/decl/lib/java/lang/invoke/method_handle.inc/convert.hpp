#pragma once

#include "decl/lib/java/lang/invoke/method_handle.hpp"

namespace mh {

template<typename T0, typename T1>
[[nodiscard]] inline bool is_type_convertible(
	c& t0, c& t1
) {
	if constexpr(same_as<T0, void_t> || same_as<T1, void_t>) {
		return true;
	}
	/* If T0 and T1 are references */
	if constexpr(same_as<T0, reference> && same_as<T1, reference>) {
		return can_cast(t0, t1);
	}
	/* If T0 and T1 are primitives */
	if constexpr(widening_conversion_allowed<T0, T1>) {
		return true;
	}
	if constexpr(same_as<T0, T1>) {
		return true;
	}
	/* If T0 is a primitive and T1 a reference */
	if constexpr(
		not_same_as<void_t, T0> &&
		not_same_as<T0, reference> &&
		same_as<T1, reference>
	) {
		return
			t1.is(object_class.get()) ||
			t1.is(wrapper_class_by_primitive_type<T0>());
	}
	/* If T0 is a reference and T1 a primitive, an unboxing conversion will be
	   applied at runtime, possibly followed by a Java method invocation
	   conversion (JLS 5.3) on the primitive value. */
	if constexpr(
		not_same_as<T1, void_t> &&
		same_as<T0, reference> &&
		not_same_as<T1, reference>
	) {
		return
		/* In the case where T0 is Object, these are the conversions allowed
		   by java.lang.reflect.Method.invoke. */
			t0.is(object_class.get()) ||
			t0.is(wrapper_class_by_primitive_type<T1>());
	}

	return false;
}

[[nodiscard]] inline bool is_type_convertible(
	c& t0, c& t1
) {
	return t0.view_raw_type([&]<typename T0>() -> bool {
		return t1.view_raw_type([&]<typename T1>() -> bool {
			return is_type_convertible<T0, T1>(
				t0, t1
			);
		});
	});
}

[[nodiscard]] inline bool is_convertible(
	o<jl::i::method_type>& new_mt,
	o<jl::i::method_type>& ori_mt
) {
	auto new_params = new_mt.parameter_types_view();
	auto ori_params = ori_mt.parameter_types_view();

	if(new_params.size() != ori_params.size()) {
		return false;
	}

	for(nuint i = 0; i < new_params.size(); ++i) {
		c& new_p = new_params[i];
		c& ori_p = ori_params[i];

		if(!is_type_convertible(new_p, ori_p)) {
			return false;
		}
	}

	c& new_ret = new_mt.return_type();
	c& ori_ret = ori_mt.return_type();

	return is_type_convertible(ori_ret, new_ret);
}

template<typename T0, typename T1>
[[nodiscard]] inline optional<reference>
try_convert_and_save_on_stack(
	T0& t0
) {
	/* If T0 and T1 are references */
	if constexpr(same_as<reference, T0> && same_as<reference, T1>) {
		return {};
	}
	/* If T0 and T1 are primitives */
	if constexpr(widening_conversion_allowed<T0, T1>) {
		if constexpr(
			not_same_as<T0, void_t>
		) {
			stack.emplace_back((T1) t0);
			return {};
		}
	}
	if constexpr( // both primitives of the same type, skip
		not_same_as<reference, T0> && not_same_as<reference, T1> &&
		same_as<T0, T1>
	) {
		return {};
	}
	/* If T0 is a primitive and T1 a reference, a Java casting
	   conversion (JLS 5.5) is applied if one exists. */
	if constexpr(
		not_same_as<reference, T0> && same_as<reference, T1>
	) {
		// TODO clang is dumb? or me? (probably second)
		if constexpr(not_same_as<reference, T0>) {
			expected<reference, reference> possible_result = try_create_object(
				wrapper_constructor_by_primitive_type<T0>(), t0
			);

			if(possible_result.is_unexpected()) {
				return possible_result.move_unexpected();
			}

			stack.emplace_back(possible_result.move_expected());
			return {};
		}
	}
	/* If T0 is a reference and T1 a primitive, an unboxing conversion
	   will be applied at runtime, possibly followed by a Java method
	   invocation conversion (JLS 5.3) on the primitive value. */
	if constexpr(
		same_as<T0, reference> && not_same_as<T1, reference>
	) {
		T1& unboxed_value = t0->template get<T1>(
			wrapper_value_field_position_by_primitive_type<T1>()
		);
		stack.emplace_back(move(unboxed_value));
		return {};
	}

	posix::abort(); // impossible
}

[[nodiscard]] inline optional<reference>
try_convert_return_value_and_save_on_stack(
	c& new_ret, c& ori_ret
) {
	bool ori_is_void = ori_ret.is(void_class.get());
	bool new_is_void = new_ret.is(void_class.get());

	if(ori_is_void && new_is_void) {
		return {};
	}

	/* If the return type T1 is marked as void, any returned value is
	   discarded */
	if(new_is_void && !ori_is_void) {
		ori_ret.view_raw_type_non_void([]<typename T0>() {
			stack.pop_back<T0>();
		});
		return {};
	}
	/* If the return type T0 is void and T1 a reference, a null value is
	   introduced. */
	/* If the return type T0 is void and T1 a primitive, a zero value is
	   introduced. */
	if(ori_is_void && !new_is_void) {
		stack.emplace_back(reference{});
		new_ret.view_raw_type_non_void([]<typename T1>() {
			stack.emplace_back(T1{});
		});
		return {};
	}

	return ori_ret.view_raw_type_non_void(
	[&]<typename T0>() -> optional<reference> {
		T0 t0 = stack.pop_back<T0>();
	
		return new_ret.view_raw_type_non_void([&]<typename T1>()
		-> optional<reference> {
			return try_convert_and_save_on_stack<
				T0, T1
			>(t0);
		});
	});
}

template<range_of<c&> NewArgs, range_of<c&> OriArgs>
[[nodiscard]] inline optional<reference>
try_convert_arguments_on_stack(
	NewArgs&& new_args, OriArgs&& ori_args, nuint index
) {
	--index;

	c& new_a = new_args[index];
	c& ori_a = ori_args[index];

	return new_a.view_raw_type_non_void([&]<typename T0>()
	-> optional<reference> {
		T0 t0 = stack.pop_back<T0>();

		if(index > 0) {
			optional<reference> possible_throwable
				= try_convert_arguments_on_stack(
					forward<NewArgs>(new_args),
					forward<OriArgs>(ori_args),
					index
				);
			if(possible_throwable.has_value()) {
				return possible_throwable.move();
			}
		}

		return ori_a.view_raw_type_non_void(
			[&]<typename T1>() -> optional<reference> {
				optional<reference> possible_thowable
				= try_convert_and_save_on_stack<T0, T1>(t0);

				if(possible_thowable.has_value()) {
					return possible_thowable.move();
				}
				return {};
			}
		);
	});
}

template<range_of<c&> NewArgs, range_of<c&> OriArgs>
[[nodiscard]] inline optional<reference>
try_convert_arguments_on_stack(
	NewArgs&& new_args, OriArgs&& ori_args
) {
	auto size = range_size(new_args);
	if(size == 0) return {};

	return try_convert_arguments_on_stack(
		forward<NewArgs>(new_args),
		forward<OriArgs>(ori_args),
		size
	);
}

}
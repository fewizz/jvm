#pragma once

#include "./conversion.inc/identity.hpp"
#include "./conversion.inc/widening_primitive.hpp"
#include "./conversion.inc/widening_reference.hpp"
#include "./conversion.inc/boxing.hpp"
#include "./conversion.inc/unboxing.hpp"

#include "./conversion.inc/contexts.hpp"

template<typename Context, typename From, typename To>
[[nodiscard]] inline bool is_conversion_allowed(
	c& from, c& to
) {
	return
	Context::conversions::
	view_first_satisfying_predicate_or_default(
		[&]<typename Conversion>() { // predicate
			if constexpr(Conversion::template are_types_allowed<From, To>) {
				return Conversion::template is_allowed<From, To>(from, to);
			}
			return false;
		},
		[&]<typename Conversion>() -> bool {
			return true;
		},
		[]() -> bool {
			return false;
		}
	);
}

template<typename Context>
[[nodiscard]] inline bool is_conversion_allowed(
	c& from, c& to
) {
	return from.view_raw_type([&]<typename From>() -> bool {
		return to.view_raw_type([&]<typename To>() -> bool {
			return is_conversion_allowed<Context, From, To>(
				from, to
			);
		});
	});
}

template<typename Context, typename From, typename To>
[[nodiscard]] inline optional<reference>
try_convert_on_stack(
	From from_value, c& from, c& to
) {
	return
	method_handle_invoke_context::conversions::
	view_first_satisfying_predicate_or_default(
		[&]<typename Conversion>() { // predicate
			if constexpr(Conversion::template are_types_allowed<From, To>) {
				return Conversion::template is_allowed<From, To>(from, to);
			}
			return false;
		},
		[&]<typename Conversion>() -> optional<reference> {
			if constexpr(
				Conversion::template are_types_allowed<From, To>
			) {
				return Conversion::template try_on_stack<From, To>(
					move(from_value), to
				);
			}
			posix::abort();
		},
		[]() -> optional<reference> {
			posix::abort();
		}
	);
}

template<typename Context, typename From>
[[nodiscard]] inline optional<reference> try_convert_on_stack(
	From from_value, c& from, c& to
) {
	return to.view_raw_type([&]<typename To>() -> optional<reference> {
		return try_convert_on_stack<Context, From, To>(
			from_value, from, to
		);
	});
}
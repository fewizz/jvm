#pragma once

#include "decl/primitives.hpp"
#include "decl/execution/stack.hpp"

struct widening_primitive_conversion {

	template<typename From, typename To>
	static constexpr bool are_types_allowed =
		(
			same_as<From, int8> &&
			same_as_any<To, int16, int32, int64, float, double>
		) ||
		(
			same_as<From, int16> &&
			same_as_any<To, int32, int64, float, double>
		) ||
		(
			same_as<From, uint16> &&
			same_as_any<To, int32, int64, float, double>
		) ||
		( same_as<From, int32> && same_as_any<To, int64, float, double>) ||
		( same_as<From, int64> && same_as_any<To, float, double>) ||
		( same_as<From, float> && same_as_any<To, double>);
	
	template<primitive From, primitive To>
	requires are_types_allowed<From, To>
	static inline bool is_allowed(c&, c&) {
		return true;
	}

	template<primitive From, primitive To>
	static inline optional<reference> try_on_stack(
		From from,
		[[maybe_unused]] c& to
	) {
		stack.emplace_back((To) from);
		return {};
	}

};
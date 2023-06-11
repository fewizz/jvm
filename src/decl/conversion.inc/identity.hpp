#pragma once

#include "decl/class.hpp"
#include "decl/execution/stack.hpp"

struct identity_conversion {

	template<typename From, typename To>
	static constexpr bool are_types_allowed
		= same_as<From, To> && (
			(same_as<reference, From> || primitive<From>) ||
			(same_as<reference, To> || primitive<To>)
		);
	
	template<same_as<reference> From, same_as<reference> To>
	static inline bool is_allowed(c& from, c& to) {
		return from.is(to);
	}

	template<primitive From, primitive To>
	static inline bool is_allowed(c&, c&) {
		return same_as<From, To>;
	}

	template<typename From, typename To>
	static inline optional<reference> try_on_stack(
		From from,
		[[maybe_unused]] c& to
	) {
		stack.emplace_back(move(from));
		return {};
	}

};
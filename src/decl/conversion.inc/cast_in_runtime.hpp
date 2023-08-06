#pragma once

#include "decl/reference.hpp"
#include "decl/execute.hpp"

struct cast_in_runtime_conversion {

	template<typename From, typename To>
	static constexpr bool are_types_allowed
		= same_as<From, reference> && same_as<To, reference>;
	
	template<same_as<reference> From, same_as<reference> To>
	static inline bool is_allowed(
		[[maybe_unused]] c& from,
		[[maybe_unused]] c& to
	) {
		return true;
	}

	template<same_as<reference> From, same_as<reference> To>
	static inline optional<reference> try_on_stack(
		[[maybe_unused]] reference from,
		[[maybe_unused]] c& to
	) {
		optional<reference> possible_throwable = try_check_cast(from, to);
		if(possible_throwable.has_value()) {
			return possible_throwable.move();
		}
		stack.emplace_back(move(from));
		return {};
	}

};
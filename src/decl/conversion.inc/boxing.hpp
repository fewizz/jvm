#pragma once

#include "decl/reference.hpp"
#include "decl/primitives.hpp"
#include "decl/class.hpp"
#include "decl/execution/stack.hpp"

#include <expected.hpp>

struct boxing_conversion {

	template<typename From, typename To>
	static constexpr bool are_types_allowed
		= primitive<From> && same_as<reference, To>;
	
	template<primitive From, same_as<reference> To>
	static inline bool is_allowed(c&, c& to) {
		return wrapper_class_by_primitive_type<From>().is(to);
	}

	template<primitive From, same_as<reference> To>
	static inline optional<reference> try_on_stack(
		From from,
		[[maybe_unused]] c& to
	) {
		instance_method& constructor
			= wrapper_constructor_by_primitive_type<From>();
		expected<reference, reference> possible_value
			= try_create_object(constructor, from);
		if(possible_value.is_unexpected()) {
			return possible_value.move_unexpected();
		}
		reference value = possible_value.move_expected();
		stack.emplace_back(move(value));
		return {};
	}

	struct followed_by_widening_reference_conversion {

		template<typename From, typename To>
		static constexpr bool are_types_allowed
			= primitive<From> && same_as<reference, To>;
		
		template<primitive From, same_as<reference> To>
		static inline bool is_allowed(c&, c& to) {
			return wrapper_class_by_primitive_type<From>().is_sub_of(
				to
			);
		}

		template<primitive From, same_as<reference> To>
		static inline optional<reference> try_on_stack(
			From from,
			c& to
		) {
			return boxing_conversion::try_on_stack<From, To>(from, to);
		}

	};

};
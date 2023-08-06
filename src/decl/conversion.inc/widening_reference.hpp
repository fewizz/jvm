#pragma once

#include "./widening_primitive.hpp"
#include "decl/object.hpp"
#include "decl/primitives.hpp"
#include "decl/lib/java/lang/null_pointer_exception.hpp"

struct widening_reference_conversion {

	template<typename From, typename To>
	static constexpr bool are_types_allowed
		= same_as<From, reference> && same_as<To, reference>;
	
	template<same_as<reference> From, same_as<reference> To>
	static inline bool is_allowed(c& from, c& to) {
		return from.is_sub_of(to);
	}

	template<same_as<reference> From, primitive To>
	static inline optional<reference> try_on_stack(
		reference from,
		[[maybe_unused]] c& to
	) {
		stack.emplace_back(move(from));
		return {};
	}

	struct followed_by_unboxig_conversion {

		template<typename From, typename To>
		static constexpr bool are_types_allowed
			= same_as<From, reference> && primitive<To>;
		
		template<same_as<reference> From, primitive To>
		static inline bool is_allowed(c& from, c&) {
			c& wrapper_c = wrapper_class_by_primitive_type<To>();
			return from.is_sub_of(wrapper_c);
		}

		template<same_as<reference> From, primitive To>
		static inline optional<reference> try_on_stack(
			reference from,
			[[maybe_unused]] c& to
		) {
			if(from.is_null()) {
				return try_create_null_pointer_exception().move();
			}
			To value = from->get<To>(
				wrapper_value_field_position_by_primitive_type<To>()
			);
			stack.emplace_back(value);
			return {};
		}

		struct followed_by_widening_primitive_conversion {

			template<typename From, typename To>
			static constexpr bool are_types_allowed
				= same_as<From, reference> && primitive<To>;

			template<same_as<reference> From, primitive To>
			static inline bool is_allowed(
				c& from,
				[[maybe_unused]] c& to
			) {
				return primitive_types::
				view_first_satisfying_predicate_or_default(
					[&]<typename FromPrimitiveType> {
						c& wrapper_c = wrapper_class_by_primitive_type<
							FromPrimitiveType
						>();
						return
							from.is_sub_of(wrapper_c) &&
							widening_primitive_conversion::
								are_types_allowed<FromPrimitiveType, To>;
					},
					[]<typename FromPrimitiveType> { return true; },
					[]{ return false; }
				);
			}

			template<same_as<reference> From, primitive To>
			static inline optional<reference> try_on_stack(
				reference from,
				[[maybe_unused]] c& to
			) {
				if(from.is_null()) {
					return try_create_null_pointer_exception().move();
				}

				primitive_types::
				view_first_satisfying_predicate_or_default(
					[&]<typename FromPrimitiveType> {
						c& wrapper_c = wrapper_class_by_primitive_type<
							FromPrimitiveType
						>();
						return
							from.c().is_sub_of(wrapper_c) &&
							widening_primitive_conversion::
								are_types_allowed<FromPrimitiveType, To>;
					},
					[&]<typename FromPrimitiveType> () {
						FromPrimitiveType value = from->get<FromPrimitiveType>(
							wrapper_value_field_position_by_primitive_type<
								FromPrimitiveType
							>()
						);

						stack.emplace_back(
							(To) value
						);
					},
					[](){}
				);

				return {};
			}

		};

	};

};
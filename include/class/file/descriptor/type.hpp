#pragma once

#include <core/span.hpp>

namespace class_file::descriptor {

	struct B{}; struct C{}; struct D{}; struct F{};
	struct I{}; struct J{}; struct S{}; struct Z{};

	struct object_type : span<uint8, uint16> {
		using base_type = span<uint8, uint16>;
		using base_type::base_type;
	};

	template<typename ComponentType>
	struct array_type;

	template<typename Type>
	static constexpr bool is_array_type = false;

	template<typename Type>
	static constexpr bool is_array_type<array_type<Type>> = true;

	template<typename Type>
	requires is_array_type<Type>
	static constexpr nuint array_type_rank = 1;

	template<typename Type>
	requires is_array_type<typename array_type<Type>::component_type>
	static constexpr nuint array_type_rank<array_type<Type>> =
		1 + array_type_rank<typename array_type<Type>::component_type>;

	template<typename ComponentType>
	struct array_type {
		using component_type = ComponentType;
		ComponentType component;

		static constexpr nuint rank =
			array_type_rank<array_type<ComponentType>>;
	};

	template<typename ComponentType>
	array_type(ComponentType) -> array_type<ComponentType>;

	static_assert(array_type<B>::rank == 1);
	static_assert(array_type<array_type<B>>::rank == 2);

};
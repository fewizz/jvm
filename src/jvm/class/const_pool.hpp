#pragma once

#include "class/file/constant.hpp"

#include <core/meta/elements/one_of.hpp>

using const_pool_entry = elements::one_of<
	class_file::constant::utf8,
	class_file::constant::int32,
	class_file::constant::float32,
	class_file::constant::int64,
	class_file::constant::float64,
	class_file::constant::_class,
	class_file::constant::string,
	class_file::constant::field_ref,
	class_file::constant::method_ref,
	class_file::constant::interface_method_ref,
	class_file::constant::name_and_type,
	class_file::constant::method_handle,
	class_file::constant::method_type,
	class_file::constant::dynamic,
	class_file::constant::invoke_dynamic,
	class_file::constant::module,
	class_file::constant::package,
	class_file::constant::skip
>;

#include "../../alloc.hpp"

#include <core/limited_list.hpp>

struct const_pool :
	private limited_list<const_pool_entry, uint16, default_allocator>
{
	using base_type = limited_list<const_pool_entry, uint16, default_allocator>;
	using base_type::base_type;

public:

	using base_type::emplace_back;
	using base_type::size;

	uint16 constants_count() const { return capacity(); }

	const auto& constant(uint16 index) const {
		return (*this)[index - 1];
	}

	auto& constant(uint16 index) {
		return (*this)[index - 1];
	}

	template<typename Type>
	Type constant(uint16 index) const {
		const const_pool_entry& entry = constant(index);
		return entry.get<Type>();
	}

	auto utf8_constant(uint16 index) const {
		return constant<class_file::constant::utf8>(index);
	}

	auto int32_constant(uint16 index) const {
		return constant<class_file::constant::int32>(index);
	}

	auto float32_constant(uint16 index) const {
		return constant<class_file::constant::float32>(index);
	}

	auto int64_constant(uint16 index) const {
		return constant<class_file::constant::int64>(index);
	}

	auto float64_constant(uint16 index) const {
		return constant<class_file::constant::float64>(index);
	}

	auto class_constant(uint16 index) const {
		return constant<class_file::constant::_class>(index);
	}

	auto string_constant(uint16 index) const {
		return constant<class_file::constant::string>(index);
	}

	auto field_ref_constant(uint16 index) const {
		return constant<class_file::constant::field_ref>(index);
	}

	auto method_ref_constant(uint16 index) const {
		return constant<class_file::constant::method_ref>(index);
	}

	auto name_and_type_constant(uint16 index) const {
		return constant<class_file::constant::name_and_type>(index);
	}
};
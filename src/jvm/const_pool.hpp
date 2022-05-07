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

#include "../alloc.hpp"

#include <core/fixed_vector.hpp>

struct const_pool : private fixed_vector<const_pool_entry, uint16, default_allocator> {
	using base_type = fixed_vector<const_pool_entry, uint16, default_allocator>;
	using base_type::base_type;

	uint16 entries_count() const { return capacity(); }

	const auto& entry(uint16 index) const { return (*this)[index - 1]; }
	auto& entry(uint16 index) { return (*this)[index - 1]; }

	template<typename Type>
	Type entry(uint16 index) const { return entry(index).template get<Type>(); }

	auto utf8_entry(uint16 index) const {
		return entry<class_file::constant::utf8>(index);
	}

	auto int32_entry(uint16 index) const {
		return entry<class_file::constant::int32>(index);
	}

	auto float32_entry(uint16 index) const {
		return entry<class_file::constant::float32>(index);
	}

	auto int64_entry(uint16 index) const {
		return entry<class_file::constant::int64>(index);
	}

	auto float64_entry(uint16 index) const {
		return entry<class_file::constant::float64>(index);
	}

	auto class_entry(uint16 index) const {
		return entry<class_file::constant::_class>(index);
	}

	auto string_entry(uint16 index) const {
		return entry<class_file::constant::string>(index);
	}

	auto field_ref_entry(uint16 index) const {
		return entry<class_file::constant::field_ref>(index);
	}

	auto method_ref_entry(uint16 index) const {
		return entry<class_file::constant::method_ref>(index);
	}

	auto name_and_type_entry(uint16 index) const {
		return entry<class_file::constant::name_and_type>(index);
	}
};
#pragma once

#include <class_file/constant.hpp>

#include <variant.hpp>
#include <list.hpp>

#include <posix/memory.hpp>

using constant = variant<
	class_file::constant::utf8,
	class_file::constant::_int,
	class_file::constant::_float,
	class_file::constant::_long,
	class_file::constant::_double,
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

struct constants :
	private list<posix::memory_for_range_of<constant>>
{
	using base_type = list<posix::memory_for_range_of<constant>>;
	using base_type::base_type;

public:

	using base_type::emplace_back;
	using base_type::size;

	uint16 constants_count() const { return capacity(); }

	const ::constant& constant(class_file::constant::index index) const {
		return (*this)[index - 1];
	}

	::constant& constant(class_file::constant::index index) {
		return (*this)[index - 1];
	}

	template<typename Type>
	Type constant(class_file::constant::index index) const {
		const ::constant& entry = constant(index);
		return entry.get<Type>();
	}

	auto utf8_constant(class_file::constant::utf8_index index) const {
		return constant<class_file::constant::utf8>(index);
	}

	auto int_constant(class_file::constant::int_index index) const {
		return constant<class_file::constant::_int>(index);
	}

	auto float_constant(class_file::constant::float_index index) const {
		return constant<class_file::constant::_float>(index);
	}

	auto long_constant(class_file::constant::long_index index) const {
		return constant<class_file::constant::_long>(index);
	}

	auto double_constant(class_file::constant::double_index index) const {
		return constant<class_file::constant::_double>(index);
	}

	auto class_constant(class_file::constant::class_index index) const {
		return constant<class_file::constant::_class>(index);
	}

	auto string_constant(class_file::constant::string_index index) const {
		return constant<class_file::constant::string>(index);
	}

	auto field_ref_constant(class_file::constant::field_ref_index index) const {
		return constant<class_file::constant::field_ref>(index);
	}

	auto method_ref_constant(
		class_file::constant::method_ref_index index
	) const {
		return constant<class_file::constant::method_ref>(index);
	}

	auto interface_method_ref_constant(
		class_file::constant::interface_method_ref_index index
	) const {
		return constant<class_file::constant::interface_method_ref>(index);
	}

	auto name_and_type_constant(
		class_file::constant::name_and_type_index index
	) const {
		return constant<class_file::constant::name_and_type>(index);
	}

	auto method_handle_constant(
		class_file::constant::method_handle_index index
	) const {
		return constant<class_file::constant::method_handle>(index);
	}

	auto invoke_dynamic_constant(
		class_file::constant::invoke_dynamic_index index
	) const {
		return constant<class_file::constant::invoke_dynamic>(index);
	}

};
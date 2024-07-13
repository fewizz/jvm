#pragma once

#include <class_file/constant.hpp>
#include <variant.hpp>
#include <posix/memory.hpp>
#include <posix/abort.hpp>

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

struct constants : initialised<posix::memory<constant, uint16>> {
	using base_type = initialised<posix::memory<constant, uint16>>;
	using base_type::base_type;

private:
	const ::constant& constant(class_file::constant::index index) const {
		return base_type::operator [] (index - 1);
	}
	::constant& constant(class_file::constant::index index) {
		return base_type::operator [] (index - 1);
	}

	template<typename Type>
	Type constant(class_file::constant::index index) const {
		const ::constant& entry = constant(index);
		return entry.get_same_as<Type>();
	}

public:

	const ::constant& operator [] (class_file::constant::index index) const {
		return constant(index);
	}

	auto operator [] (class_file::constant::utf8_index index) const {
		return constant<class_file::constant::utf8>(index);
	}

	auto operator [] (class_file::constant::name_index index) const {
		return class_file::constant::name {
			constant<class_file::constant::utf8>(index)
		};
	}

	auto operator [] (class_file::constant::int_index index) const {
		return constant<class_file::constant::_int>(index);
	}

	auto operator [] (class_file::constant::float_index index) const {
		return constant<class_file::constant::_float>(index);
	}

	auto operator [] (class_file::constant::long_index index) const {
		return constant<class_file::constant::_long>(index);
	}

	auto operator [] (class_file::constant::double_index index) const {
		return constant<class_file::constant::_double>(index);
	}

	auto operator [] (class_file::constant::class_index index) const {
		return constant<class_file::constant::_class>(index);
	}

	auto operator [] (class_file::constant::string_index index) const {
		return constant<class_file::constant::string>(index);
	}

	auto operator [] (class_file::constant::field_ref_index index) const {
		return constant<class_file::constant::field_ref>(index);
	}

	auto operator [] (class_file::constant::method_ref_index index) const {
		return constant<class_file::constant::method_ref>(index);
	}

	auto operator [] (
		class_file::constant::interface_method_ref_index index
	) const {
		return constant<class_file::constant::interface_method_ref>(index);
	}

	auto operator [] (class_file::constant::name_and_type_index index) const {
		return constant<class_file::constant::name_and_type>(index);
	}

	auto operator [] (class_file::constant::method_handle_index index) const {
		return constant<class_file::constant::method_handle>(index);
	}

	auto operator [] (class_file::constant::invoke_dynamic_index index) const {
		return constant<class_file::constant::invoke_dynamic>(index);
	}

	template<typename Handler>
	decltype(auto) view_method_or_interface_method_constant_index(
		class_file::constant::method_or_interface_method_ref_index index,
		Handler&& handler
	) {
		::constant c = constant(index);
		if(c.is_same_as<class_file::constant::method_ref>()) {
			return handler(
				class_file::constant::method_ref_index{ index }
			);
		}
		if(c.is_same_as<class_file::constant::interface_method_ref>()) {
			return handler(
				class_file::constant::interface_method_ref_index{ index }
			);
		}
		posix::abort(); // impossible
	}

	template<typename Handler>
	decltype(auto) view_method_or_interface_method_constant(
		class_file::constant::method_or_interface_method_ref_index index,
		Handler&& handler
	) {
		return view_method_or_interface_method_constant_index(
			index,
			[&](auto index) {
				return handler((*this)[index]);
			}
		);
	}

};
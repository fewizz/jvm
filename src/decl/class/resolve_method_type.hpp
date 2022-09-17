#pragma once

#include "./resolve_class.hpp"
#include "object/reference.hpp"

#include <class_file/descriptor/reader.hpp>

#include <range.hpp>

template<basic_range Descriptor>
inline reference resolve_method_type(_class& d, Descriptor&& descriptor) {
	auto get_class = [&]<typename Type>(Type t) -> _class& {
		if constexpr(same_as<Type, class_file::v>) {
			return void_class.value();
		}
		if constexpr(same_as<Type, class_file::b>) {
			return byte_class.value();
		}
		if constexpr(same_as<Type, class_file::c>) {
			return char_class.value();
		}
		if constexpr(same_as<Type, class_file::d>) {
			return double_class.value();
		}
		if constexpr(same_as<Type, class_file::f>) {
			return float_class.value();
		}
		if constexpr(same_as<Type, class_file::i>) {
			return int_class.value();
		}
		if constexpr(same_as<Type, class_file::j>) {
			return long_class.value();
		}
		if constexpr(same_as<Type, class_file::s>) {
			return short_class.value();
		}
		if constexpr(same_as<Type, class_file::z>) {
			return bool_class.value();
		}
		else if constexpr(same_as<Type, class_file::object>) {
			return resolve_class(d, t);
		}
		else if constexpr(same_as<Type, class_file::array>) {
			return resolve_class(d, t);
		}
	};

	/*if(descriptor.size() > 0 && descriptor[0] == '(') {
		class_file::descriptor::method_reader reader{ descriptor.iterator() };
		nuint paramers_count = reader.paramers_count();
		auto [ret_reader, result] = reader([]);
		ret_reader(resolve);
	}
	else {
		class_file::descriptor::read_field(descriptor.iterator(), resolve);
	}*/


}
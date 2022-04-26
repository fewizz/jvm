#pragma once

#include "alloc.hpp"
#include "class/file/constant.hpp"
#include "class/file/access_flag.hpp"

#include <core/meta/elements/one_of.hpp>

using const_pool_entry = elements::one_of<
	class_file::constant::utf8,
	class_file::constant::integer,
	class_file::constant::_float,
	class_file::constant::_long,
	class_file::constant::_double,
	class_file::constant::_class,
	class_file::constant::string,
	class_file::constant::fieldref,
	class_file::constant::methodref,
	class_file::constant::interface_methodref,
	class_file::constant::name_and_type,
	class_file::constant::method_handle,
	class_file::constant::method_type,
	class_file::constant::dynamic,
	class_file::constant::invoke_dynamic,
	class_file::constant::module,
	class_file::constant::package,
	uint8
>;

template<typename Type, typename SizeType>
struct owning_span : span<Type, SizeType> {

	owning_span(uint16 size) :
		span<Type, SizeType> {
			(Type*) malloc(sizeof(Type) * size), size
		}
	{}

	~owning_span() {
		free(this->data());
	}

};

struct field {
	class_file::access_flag access_flags;
	uint16 name_index;
	uint16 desc_index;
};

struct method {
	class_file::access_flag access_flags;
	uint16 name_index;
	uint16 desc_index;
};

struct _class {
	owning_span<const_pool_entry, uint16> const_pool;
	owning_span<field, uint16> fields;
	owning_span<method, uint16> methods;
};
#pragma once

#include "alloc.hpp"
#include "class/file/constant.hpp"
#include "class/file/access_flag.hpp"

#include <core/fixed_vector.hpp>
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
	class_file::constant::skip
>;

struct field {
	class_file::access_flags access_flags;
	uint16 name_index;
	uint16 desc_index;
};

struct code {
	uint16 max_locals;
	uint16 max_stack;
	span<uint8, uint32> code;
};

struct method {
	class_file::access_flags access_flags;
	uint16 name_index;
	uint16 desc_index;
	code code;
};

struct _class {
	fixed_vector<const_pool_entry, uint16, default_allocator> const_pool;
	class_file::access_flags access_flags;
	uint16 this_class;
	uint16 super_class;
	fixed_vector<uint16, uint16, default_allocator> interfaces;
	fixed_vector<field, uint16, default_allocator> fields;
	fixed_vector<method, uint16, default_allocator> methods;
	fixed_vector<void*, uint16, default_allocator> ptrs;
	void* data;
};
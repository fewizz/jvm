#pragma once

#include "alloc.hpp"
#include "field.hpp"
#include "./field_index.hpp"

#include <core/range.hpp>
#include <core/limited_list.hpp>
#include <core/optional.hpp>

struct _class;

struct fields_container :
	limited_list<field, uint16, default_allocator>
{
	using base_type = limited_list<field, uint16, default_allocator>;
	using base_type::base_type;

	template<range Name, range Descriptor>
	optional<field_index>
	try_find_declared_instance_field_index(
		_class& c, Name&& name, Descriptor&&
	);

};
#pragma once

#include "../alloc.hpp"
#include "field_value.hpp"
#include "instance_field_index.hpp"
#include <core/limited_list.hpp>
#include <core/integer.hpp>

struct _class;

struct object {
private:
	_class& class_;
	limited_list<field_value, uint16, default_allocator> values_;
public:
	inline object(_class& c, uint16 values_count);
	inline field_value& operator [] (instance_field_index index);
	inline _class& _class();
	inline auto& values() { return values_; }
};

#include "field.hpp"

object::object(
	::_class& c,
	uint16 values_count
) :
	class_{ c },
	values_{ values_count }
{}

field_value& object::operator [] (instance_field_index index) {
	return values_[(uint16) index];
}

_class& object::_class() { return class_; }
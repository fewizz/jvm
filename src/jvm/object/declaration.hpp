#pragma once

#include "../field/value.hpp"
#include "../field/index.hpp"
#include "../../alloc.hpp"
#include <core/meta/elements/optional.hpp>
#include <core/limited_list.hpp>
#include <core/integer.hpp>

struct _class;
struct reference;
struct field_value;

struct object {
private:
	uint32 references_ = 0;
	optional<_class&> class_;
	using values_type = limited_list<field_value, uint16, default_allocator>;
	values_type values_;

	void on_reference_added() {
		++references_;
	}

	inline void on_reference_removed();

	friend reference;

	friend reference create_object(_class& c);

public:

	inline object(_class& c);

	inline field_value& operator [] (instance_field_index index);

	_class& _class() {
		return class_.value();
	}

	auto& values() { return values_; }
	uint32 references() { return references_; }
};
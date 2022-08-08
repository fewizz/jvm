#pragma once

#include "field/value.hpp"
#include "class/instance_field_index.hpp"
#include "alloc.hpp"

#include <optional.hpp>
#include <memory_list.hpp>
#include <integer.hpp>

struct _class;
struct reference;
struct field_value;

struct object {
private:
	uint32 references_ = 0;
	optional<_class&> class_;
	using values_type = memory_list<field_value, uint16>;
	values_type values_;

	void on_reference_added();

	void on_reference_removed();

	void unsafe_decrease_reference_count_without_destroing();

	friend reference;

	friend reference create_object(_class& c);

public:

	object(_class& c);
	~object();

	field_value& operator [] (instance_field_index index);

	_class& _class() {
		return class_.value();
	}

	auto& values() { return values_; }

	uint32 references() { return references_; }

};
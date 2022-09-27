#pragma once

#include "field/value.hpp"
#include "class/field_index.hpp"

#include <optional.hpp>
#include <integer.hpp>
#include <list.hpp>

#include <posix/memory.hpp>

struct _class;
struct reference;
struct field_value;

struct object {
private:
	uint32 references_ = 0;
	optional<_class&> class_;
	list<posix::memory_for_range_of<field_value>> values_;

	void on_reference_added();

	void on_reference_removed();

	void unsafe_decrease_reference_count_without_destroing();

	friend reference;

	friend reference create_object(_class& c);

public:

	object(_class& c);
	~object();

	field_value& operator [] (instance_field_index index);

	const ::_class& _class() const { return class_.value(); }
	      ::_class& _class()       { return class_.value(); }

	auto& values() { return values_; }

	uint32 references() { return references_; }

};
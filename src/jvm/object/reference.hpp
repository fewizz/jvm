#pragma once

#include <core/integer.hpp>
#include <core/meta/elements/optional.hpp>

struct counted_object_ptr;
struct object;
struct _class;

struct reference {
private:
	optional<object&> obj_;

	friend reference create_object(_class& c);

public:

	reference(object& obj) :
		obj_{ obj }
	{}

	reference() {};

	inline reference(const reference&);
	inline reference(reference&&);

	inline reference& operator = (const reference&);
	inline reference& operator = (reference&&);

	inline object& object();
	inline ~reference();

	inline bool is_null() const;
};

#include "declaration.hpp"
#include "../../alloc.hpp"
#include "../../abort.hpp"
#include <core/exchange.hpp>
#include <stdio.h>

reference::reference(const reference& other) :
	obj_{ other.obj_ }
{
	if(obj_.has_value()) {
		obj_.value().on_reference_added();
	}
}

reference::reference(reference&& other) :
	obj_{ exchange(other.obj_, elements::none{}) }
{}

reference& reference::operator = (const reference& other) {
	if(obj_.has_value()) {
		obj_.value().on_reference_removed();
	}
	obj_ = other.obj_;
	if(obj_.has_value()) {
		obj_.value().on_reference_added();
	}
	return *this;
}

reference& reference::operator = (reference&& other) {
	if(obj_.has_value()) {
		obj_.value().on_reference_removed();
	}
	obj_ = { exchange(other.obj_, elements::none{}) };
	return *this;
}

object& reference::object() {
	if(!obj_.has_value()) {
		fprintf(stderr, "obj_ is nullptr");
		abort();
	}
	return obj_.value();
}

reference::~reference() {
	if(obj_.has_value()) {
		obj_.value().on_reference_removed();
	}
}

inline bool reference::is_null() const {
	return !obj_.has_value();
}
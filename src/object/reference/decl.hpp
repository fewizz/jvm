#pragma once

#include <core/integer.hpp>

struct object;
struct _class;

struct reference {
private:
	object* obj_{};

	friend reference create_object(_class& c);

public:

	reference(object& obj);

	reference() {};

	~reference();

	reference(const reference&);
	reference(reference&&);

	reference& operator = (const reference&);
	reference& operator = (reference&&);

	::object& object();
	::object* object_ptr() { return obj_; }

	::object* operator -> () { return obj_; }

	bool is_null() const;
};

static_assert(sizeof(reference) == sizeof(void*));
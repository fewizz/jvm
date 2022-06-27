#pragma once

#include <core/integer.hpp>

struct object;
struct _class;

struct reference {
private:
	object* obj_{};

	friend reference create_object(_class& c);

public:

	inline reference(object& obj);

	reference() {};

	inline reference(const reference&);
	inline reference(reference&&);

	inline reference& operator = (const reference&);
	inline reference& operator = (reference&&);

	inline ::object& object();
	inline ::object* object_ptr() { return obj_; }
	inline ~reference();

	inline bool is_null() const;
};
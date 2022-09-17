#pragma once

#include <integer.hpp>
#include <posix/memory.hpp>

struct object;
struct _class;

struct reference {
private:
	object* obj_ = nullptr;

	friend reference create_object(_class& c);

public:

	reference(object& obj);

	reference() = default;

	~reference();
	object& unsafe_release_without_destroing();

	reference(const reference&);
	reference(reference&&);

	reference& operator = (const reference&);
	reference& operator = (reference&&);

	::object& object();
	::object* object_ptr() { return obj_; }

	::object* operator -> () { return obj_; }

	bool is_null() const;

	operator const ::object& () const { return *obj_; }
	operator       ::object& ()       { return *obj_; }
};
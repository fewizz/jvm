#pragma once

#include <integer.hpp>
#include <posix/memory.hpp>

struct object;
struct _class;
struct field_value;

struct reference {
private:
	object* obj_ptr_ = nullptr;

	friend reference create_object(_class& c);

public:

	reference(object& obj);

	reference() = default;

	~reference();
	object& unsafe_release_without_destroing();

	reference(const reference& );
	reference(      reference&&);

	reference& operator = (const reference& );
	reference& operator = (      reference&&);

	const ::object& object() const;
	      ::object& object();

	const ::object* object_ptr() const;
	      ::object* object_ptr();

	::object* operator -> () { return object_ptr(); }

	const field_value& operator [] (uint16 index) const;
	      field_value& operator [] (uint16 index);

	bool is_null() const;

	const ::_class& _class() const;
	      ::_class& _class()      ;

	operator const ::object& () const { return *object_ptr(); }
	operator       ::object& ()       { return *object_ptr(); }
};
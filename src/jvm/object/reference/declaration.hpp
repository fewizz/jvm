#pragma once

#include <core/integer.hpp>
#include <core/meta/elements/optional.hpp>

struct object;
struct _class;

struct reference {
private:
	optional<object&> obj_;

	friend reference create_object(_class& c);

public:

	inline reference(object& obj);

	reference() {};

	inline reference(const reference&);
	inline reference(reference&&);

	inline reference& operator = (const reference&);
	inline reference& operator = (reference&&);

	inline object& object();
	inline ~reference();

	inline bool is_null() const;
};
#pragma once

#include <integer.hpp>
#include <expected.hpp>
#include <posix/memory.hpp>

struct object;
struct _class;

struct reference {
private:
	object* obj_ptr_ = nullptr;

	friend expected<reference, reference> try_create_object(_class& c);

	void reset();

	void try_reset() {
		if(obj_ptr_ != nullptr) {
			reset();
		}
	}

public:

	reference(object& obj);

	reference() = default;

	~reference();
	object& unsafe_release_without_destroing();

	reference(const reference& );
	reference(      reference&&);

	reference& operator = (const reference& );
	reference& operator = (      reference&&);

	reference operator = (decltype(nullptr)) {
		try_reset();
		return *this;
	}

	const ::object& object() const;
	      ::object& object();

	const ::object* object_ptr() const;
	      ::object* object_ptr();

	::object* operator -> () { return object_ptr(); }

	bool is_null() const;

	const ::_class& _class() const;
	      ::_class& _class()      ;

	operator const ::object& () const & { return *object_ptr(); }
	operator       ::object& ()       & { return *object_ptr(); }
};

static inline reference nullptr_ref{};
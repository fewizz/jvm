#pragma once

#include <integer.hpp>
#include <expected.hpp>
#include <posix/memory.hpp>

struct c;

struct object;

struct reference {
private:
	object* obj_ptr_ = nullptr;

	friend expected<reference, reference> try_create_object(::c& c);

	void reset();

	void try_reset() {
		if(obj_ptr_ != nullptr) {
			reset();
		}
	}

public:

	reference(object& obj);
	reference(object* obj_ptr);

	reference() = default;

	~reference();
	object& unsafe_release_without_destroing();

	reference(const reference& );
	reference(      reference&&);

	reference& operator = (const reference& );
	reference& operator = (      reference&&);

	reference& operator = (object&);

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

	const ::c& c() const;
	      ::c& c()      ;

	operator const ::object& () const & { return *object_ptr(); }
	operator       ::object& ()       & { return *object_ptr(); }
};

static inline reference nullptr_ref{};
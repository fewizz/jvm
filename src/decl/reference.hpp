#pragma once

#include <integer.hpp>
#include <expected.hpp>
#include <posix/memory.hpp>

struct c;

template<typename Type>
struct object_of;

namespace jl {
	struct object;
}

struct reference {
private:
	object_of<jl::object>* obj_ptr_ = nullptr;

	friend expected<reference, reference> try_create_object(::c& c);

	void reset();

	void try_reset() {
		if(obj_ptr_ != nullptr) {
			reset();
		}
	}

public:

	reference(object_of<jl::object>& obj);
	reference(object_of<jl::object>* obj_ptr);

	reference() = default;

	~reference();
	object_of<jl::object>& unsafe_release_without_destroing();

	reference(const reference& );
	reference(      reference&&);

	reference& operator = (const reference& );
	reference& operator = (      reference&&);

	reference& operator = (object_of<jl::object>&);

	reference operator = (decltype(nullptr)) {
		try_reset();
		return *this;
	}

	const object_of<jl::object>& object() const;
	      object_of<jl::object>& object();

	const object_of<jl::object>* object_ptr() const;
	      object_of<jl::object>* object_ptr();

	object_of<jl::object>* operator -> () { return object_ptr(); }

	bool is_null() const;

	const ::c& c() const;
	      ::c& c()      ;

	operator const object_of<jl::object>& () const & { return *object_ptr(); }
	operator       object_of<jl::object>& ()       & { return *object_ptr(); }
};

static inline reference nullptr_ref{};
#pragma once

#include <integer.hpp>
#include <expected.hpp>
#include <posix/memory.hpp>

struct c;

namespace jl {
	struct object;
}

template<typename Type>
struct o;

struct reference {
private:
	o<jl::object>* obj_ptr_ = nullptr;

	friend expected<reference, reference> try_create_object(::c& c);

	void reset();

	void try_reset() {
		if(obj_ptr_ != nullptr) {
			reset();
		}
	}

public:

	reference(o<jl::object>& obj);
	reference(o<jl::object>* obj_ptr);

	reference() = default;

	~reference();
	o<jl::object>& unsafe_release_without_destroing();

	reference(const reference& );
	reference(      reference&&);

	reference& operator = (const reference& );
	reference& operator = (      reference&&);

	reference& operator = (o<jl::object>&);

	reference operator = (decltype(nullptr)) {
		try_reset();
		return *this;
	}

	const o<jl::object>& object() const;
	      o<jl::object>& object();

	const o<jl::object>* object_ptr() const;
	      o<jl::object>* object_ptr();

	o<jl::object>* operator -> () { return object_ptr(); }

	bool is_null() const;

	const ::c& c() const;
	      ::c& c()      ;

	operator const o<jl::object>& () const & { return *object_ptr(); }
	operator       o<jl::object>& ()       & { return *object_ptr(); }
};

static inline reference nullptr_ref{};
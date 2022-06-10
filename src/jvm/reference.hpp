#pragma once

#include <core/integer.hpp>

struct counted_object_ptr;
struct object;

struct reference {
private:
	counted_object_ptr* ptr_;
	inline void decrement_refernce();
public:
	inline reference() = default;
	inline reference(counted_object_ptr&);

	inline reference(const reference&);
	inline reference(reference&&);

	inline reference& operator = (const reference&);
	inline reference& operator = (reference&&);

	inline object& object();
	inline ~reference();

	inline bool is_null() const;
};

#include "counted_object_ptr.hpp"
#include "../alloc.hpp"
#include "../abort.hpp"
#include <core/exchange.hpp>
#include <stdio.h>

inline void reference::decrement_refernce() {
	if(ptr_ != nullptr && ptr_->count >= 1) {
		--(ptr_->count);
		if(ptr_->count == 0) {
			free(ptr_->object_ptr);
			ptr_->count = 0;
			ptr_->object_ptr = nullptr;
			ptr_ = nullptr;
		}
	}
}

reference::reference(counted_object_ptr& ptr) :
	ptr_{ &ptr }
{
	++(ptr_->count);
}

reference::reference(const reference& other) :
	ptr_{ other.ptr_ }
{
	if(ptr_ != nullptr) {
		++(ptr_->count);
	}
}

reference::reference(reference&& other) :
	ptr_{ exchange(other.ptr_, nullptr) }
{}

reference& reference::operator = (const reference& other) {
	decrement_refernce();
	ptr_ = other.ptr_;
	if(ptr_ != nullptr) {
		++(ptr_->count);
	}
	return *this;
}

reference& reference::operator = (reference&& other) {
	decrement_refernce();
	ptr_ = { exchange(other.ptr_, nullptr) };
	return *this;
}

object& reference::object() {
	if(ptr_ == nullptr) {
		fprintf(stderr, "ptr_ is nullptr");
		abort();
	}
	if(ptr_->object_ptr == nullptr) {
		fprintf(stderr, "ptr_->object_ptr is nullptr");
		abort();
	}
	return *(ptr_->object_ptr);
}

reference::~reference() {
	decrement_refernce();
}

inline bool reference::is_null() const {
	return ptr_ == nullptr || ptr_->object_ptr == nullptr;
}
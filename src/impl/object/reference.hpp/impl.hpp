#include "object/reference.hpp"

#include "object.hpp"
#include "alloc.hpp"
#include "abort.hpp"

#include <core/exchange.hpp>

#include <stdio.h>

inline reference::reference(::object& obj) : obj_{ &obj } {
	obj.on_reference_added();
}

inline reference::reference(const reference& other) :
	obj_{ other.obj_ }
{
	if(obj_ != nullptr) {
		obj_->on_reference_added();
	}
}

inline reference::reference(reference&& other) :
	obj_{ exchange(other.obj_, nullptr) }
{}

inline reference& reference::operator = (const reference& other) {
	::object* prev = obj_; // in case if assigning to self
	obj_ = other.obj_;
	if(obj_ != nullptr) {
		obj_->on_reference_added();
	}
	if(prev != nullptr) {
		prev->on_reference_removed();
	}
	return *this;
}

inline reference& reference::operator = (reference&& other) {
	::object* prev = obj_; // in case if assigning to self
	obj_ = exchange(other.obj_, nullptr);
	if(prev != nullptr) {
		prev->on_reference_removed();
	}
	return *this;
}

inline object& reference::object() {
	if(obj_ == nullptr) {
		fprintf(stderr, "obj_ is nullptr");
		abort();
	}
	return *obj_;
}

inline reference::~reference() {
	if(obj_ != nullptr) {
		/*fprintf(
			stderr,
			"reference destruction with object address = %p\n",
			obj_
		);*/
		exchange(obj_, nullptr)->on_reference_removed();
	}
}

inline ::object& reference::unsafe_release_without_destroing() {
	obj_->unsafe_decrease_reference_count_without_destroing();
	return *exchange(obj_, nullptr);
}

inline bool reference::is_null() const {
	return obj_ == nullptr;
}
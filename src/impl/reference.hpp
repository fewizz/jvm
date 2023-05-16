#include "decl/reference.hpp"

#include "decl/object.hpp"
#include "decl/execution/info.hpp"

#include <exchange.hpp>

#include <print/print.hpp>

inline void reference::reset() {
	object().on_reference_removed();
	obj_ptr_ = nullptr;
}

inline reference::reference(::object& obj) : obj_ptr_{ &obj }{
	obj.on_reference_added();
}

inline reference::reference(::object* obj_ptr) : obj_ptr_{ obj_ptr } {
	if(obj_ptr != nullptr) {
		obj_ptr->on_reference_added();
	}
}

inline const ::object* reference::object_ptr() const { return obj_ptr_; }
inline       ::object* reference::object_ptr()       { return obj_ptr_; }

inline reference::reference(const reference& other) :
	obj_ptr_{ other.obj_ptr_ }
{
	if(obj_ptr_ != nullptr) {
		object().on_reference_added();
	}
}

inline reference::reference(reference&& other) :
	obj_ptr_{ exchange(other.obj_ptr_, nullptr) }
{}

inline reference& reference::operator = (const reference& other) {
	if(this == &other) {
		return *this;
	}
	try_reset();
	obj_ptr_ = other.obj_ptr_;
	if(obj_ptr_ != nullptr) {
		object().on_reference_added();
	}
	return *this;
}

inline reference& reference::operator = (reference&& other) {
	if(this == &other) {
		return *this;
	}
	try_reset();
	obj_ptr_ = exchange(other.obj_ptr_, nullptr);
	return *this;
}

inline reference& reference::operator = (object_of<jl::object>& o) {
	if(&o == obj_ptr_) {
		return *this;
	}
	try_reset();
	obj_ptr_ = &o;
	o.on_reference_added();
	return *this;
}

inline void abort_if_null(const object* obj_ptr) {
	if(obj_ptr == nullptr) {
		print::err("obj_ is nullptr\n");
		posix::abort();
	}
}

inline const object& reference::object() const {
	abort_if_null(object_ptr());
	return *object_ptr();
}

inline object& reference::object() {
	abort_if_null(object_ptr());
	return *object_ptr();
}

inline reference::~reference() {
	if(obj_ptr_ != nullptr) {
		/*if(info) {
			tabs();
			print("# reference destruction, object @");
			print_hex((nuint)obj_ptr_);
			print("\n");
		}*/
		object().on_reference_removed();
		obj_ptr_ = nullptr;
	}
}

inline ::object& reference::unsafe_release_without_destroing() {
	object().unsafe_decrease_reference_count_without_destroing();
	::object& o = object();
	exchange(obj_ptr_, nullptr);
	return o;
}

inline bool reference::is_null() const {
	return obj_ptr_ == nullptr;
}

inline const c& reference::c() const { return object().c(); }
inline       c& reference::c()       { return object().c(); }
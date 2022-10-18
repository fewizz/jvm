#include "decl/object/reference.hpp"

#include "decl/object.hpp"
#include "decl/print.hpp"
#include "decl/execution/info.hpp"

#include <exchange.hpp>

#include <posix/io.hpp>

inline reference::reference(::object& obj) : obj_ptr_{ &obj }{
	obj.on_reference_added();
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
	::object* prev = object_ptr();
	obj_ptr_ = other.obj_ptr_;
	if(obj_ptr_ != nullptr) {
		object().on_reference_added();
	}
	if(prev != nullptr) {
		prev->on_reference_removed();
	}
	return *this;
}

inline reference& reference::operator = (reference&& other) {
	if(this == &other) {
		return *this;
	}
	::object* prev = object_ptr(); // in case if assigning same object
	obj_ptr_ = exchange(other.obj_ptr_, nullptr);
	if(prev != nullptr) {
		prev->on_reference_removed();
	}
	return *this;
}

inline void abort_if_null(const object* obj_ptr) {
	if(obj_ptr == nullptr) {
		posix::std_err.write_from(c_string{ "obj_ is nullptr\n" });
		abort();
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

inline const field_value& reference::operator [] (uint16 index) const {
	return object()[index];
}
inline       field_value& reference::operator [] (uint16 index)       {
	return object()[index];
}

inline reference::~reference() {
	if(obj_ptr_ != nullptr) {
		if(info) {
			tabs();
			print("reference destruction, object @");
			print_hex((nuint)obj_ptr_);
			print("\n");
		}
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

inline const _class& reference::_class() const { return object()._class(); }
inline       _class& reference::_class()       { return object()._class(); }
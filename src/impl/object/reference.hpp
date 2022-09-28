#include "decl/object/reference.hpp"

#include "decl/object.hpp"

#include <exchange.hpp>

#include <posix/io.hpp>

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
	if(this == &other) {
		return *this;
	}
	::object* prev = obj_; // in case if assigning same object
	obj_ = exchange(other.obj_, nullptr);
	if(prev != nullptr) {
		prev->on_reference_removed();
	}
	return *this;
}

inline object& reference::object() {
	if(obj_ == nullptr) {
		posix::std_err.write_from(c_string{ "obj_ is nullptr\n" });
		abort();
	}
	return *obj_;
}

inline field_value& reference::operator [] (uint16 index) const {
	return obj_->operator [] (index);
}
inline field_value& reference::operator [] (uint16 index)       {
	return obj_->operator [] (index);
}

inline reference::~reference() {
	if(obj_ != nullptr) {
		/*fprintf(
			stderr,
			"reference destruction with object address = %p\n",
			obj_
		);*/
		obj_->on_reference_removed();
		obj_ = nullptr;
	}
}

inline ::object& reference::unsafe_release_without_destroing() {
	obj_->unsafe_decrease_reference_count_without_destroing();
	return *exchange(obj_, nullptr);
}

inline bool reference::is_null() const {
	return obj_ == nullptr;
}

inline const _class& reference::_class() const { return obj_->_class(); }
inline       _class& reference::_class()       { return obj_->_class(); }
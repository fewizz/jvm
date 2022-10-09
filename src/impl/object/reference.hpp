#include "decl/object/reference.hpp"

#include "decl/object.hpp"

#include <exchange.hpp>

#include <posix/io.hpp>

static uint64 min_address = -1;

[[gnu::constructor]] static void guess_min_address() {
	auto allocated_int = posix::allocate_memory_for<int>(1);
	min_address = (uint64) &allocated_int[0].get();
	const uint64 tt_gb = 4 * (1ull << 30);
	if(
		min_address < tt_gb ||
		(min_address & 0b111) != 0
	) {
		abort();
	}
	min_address -= tt_gb;
}

inline reference::reference(::object& obj) {
	uint64 ptr_as_uint64 = (uint64) &obj;
	if(ptr_as_uint64 < min_address) {
		abort();
	}

	uint64 shorted_ptr_as_uint64 =
		(ptr_as_uint64 - min_address) >> 3;
	uint32 shorted_ptr_as_uint32 = (uint32) shorted_ptr_as_uint64;
	if(shorted_ptr_as_uint64 != shorted_ptr_as_uint32) {
		abort();
	}
	obj_ptr_raw_ = shorted_ptr_as_uint32;
	obj.on_reference_added();
}

inline const ::object* reference::object_ptr() const {
	return (::object*)
		((((uint64) obj_ptr_raw_) << 3) + min_address);
}
inline       ::object* reference::object_ptr()       {
	return (::object*)
		((((uint64) obj_ptr_raw_) << 3) + min_address);
}

inline reference::reference(const reference& other) :
	obj_ptr_raw_{ other.obj_ptr_raw_ }
{
	if(obj_ptr_raw_ != 0) {
		object_ptr()->on_reference_added();
	}
}

inline reference::reference(reference&& other) :
	obj_ptr_raw_{ exchange(other.obj_ptr_raw_, 0) }
{}

inline reference& reference::operator = (const reference& other) {
	::object* prev = object_ptr(); // in case if assigning to self
	uint32 obj_ptr_raw_prev = obj_ptr_raw_;
	obj_ptr_raw_ = other.obj_ptr_raw_;
	if(obj_ptr_raw_ != 0) {
		object_ptr()->on_reference_added();
	}
	if(obj_ptr_raw_prev != 0) {
		prev->on_reference_removed();
	}
	return *this;
}

inline reference& reference::operator = (reference&& other) {
	if(this == &other) {
		return *this;
	}
	::object* prev = object_ptr(); // in case if assigning same object
	uint32 obj_ptr_raw_prev = obj_ptr_raw_;
	obj_ptr_raw_ = exchange(other.obj_ptr_raw_, 0);
	if(obj_ptr_raw_prev != 0) {
		prev->on_reference_removed();
	}
	return *this;
}

inline const object& reference::object() const {
	if(obj_ptr_raw_ == 0) {
		posix::std_err.write_from(c_string{ "obj_ is nullptr\n" });
		abort();
	}
	return *object_ptr();
}

inline object& reference::object() {
	if(obj_ptr_raw_ == 0) {
		posix::std_err.write_from(c_string{ "obj_ is nullptr\n" });
		abort();
	}
	return *object_ptr();
}

inline const field_value& reference::operator [] (uint16 index) const {
	return object()[index];
}
inline       field_value& reference::operator [] (uint16 index)       {
	return object()[index];
}

inline reference::~reference() {
	if(obj_ptr_raw_ != 0) {
		/*fprintf(
			stderr,
			"reference destruction with object address = %p\n",
			obj_
		);*/
		object().on_reference_removed();
		obj_ptr_raw_ = 0;
	}
}

inline ::object& reference::unsafe_release_without_destroing() {
	object().unsafe_decrease_reference_count_without_destroing();
	::object& o = object();
	exchange(obj_ptr_raw_, 0);
	return o;
}

inline bool reference::is_null() const {
	return obj_ptr_raw_ == 0;
}

inline const _class& reference::_class() const { return object()._class(); }
inline       _class& reference::_class()       { return object()._class(); }
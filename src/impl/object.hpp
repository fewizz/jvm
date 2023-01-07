#include "decl/object.hpp"

#include "decl/mutex_attribute_recursive.hpp"
#include "decl/class.hpp"
#include "decl/field.hpp"
#include "decl/array.hpp"

#include "execution/info.hpp"

#include <posix/memory.hpp>
#include <posix/io.hpp>

inline object::object(::_class& c) :
	class_{ c },
	mutex_ { [&]() {
		return posix::create_mutex(get_mutex_attribute_recursive());
	}()},
	data_ {
		posix::allocate_memory_for<uint8>(c.instance_layout().size())
	}
{
	if(info) {
		tabs();
		print::out("# constructing object @");
		print::out.hex((nuint)this);
		auto name = _class().name();
		print::out(" of type ", name, "\n");
	}

	c.instance_fields().for_each_index([&](nuint field_index) {
		view_ptr(field_index, []<typename Type>(Type* ptr) {
			new (ptr) Type();
		});
	});
}

inline object::~object() {
	if(info) {
		tabs();
		print::out("# destructing object @");
		print::out.hex((nuint)this);
		auto name = _class().name();
		print::out(" of type ", name, "\n");
	}

	if(_class().is_array()) {
		uint8* data = array_data<uint8>(*this);
		if(_class().get_component_class().is_not_primitive()) {
			for(nuint x = array_length(*this); x > 0; --x) {
				((reference*) data)[x - 1].~reference();
			}
		}
		posix::free_raw_memory(data);
	}

	class_->instance_fields().for_each_index([&](nuint field_index) {
		view(field_index, []<typename Type>(Type& e) {
			e.~Type();
		});
	});
}

inline void object::on_reference_added() {
	++references_;
	if(info) {
		tabs();
		print::out("# added reference to object @");
		print::out.hex((nuint)this);
		auto name = _class().name();
		print::out(" of type ", name, ", ", references_, " in sum\n");
	}
}

inline void object::on_reference_removed() {
	if(references_ == 0) {
		print::err("# removing reference on object without references\n");
		posix::abort();
	}
	--references_;
	if(info) {
		tabs();
		print::out("# removed reference to object @");
		print::out.hex((nuint)this);
		auto name = _class().name();
		print::out(" of type ", name, ", ", references_, " left\n");
	}
	if(references_ == 0) {
		uint8* ptr_to_this = (uint8*) this;
		this->~object();
		posix::free_raw_memory(ptr_to_this);
	}
}

inline void object::unsafe_decrease_reference_count_without_destroing() {
	if(references_ == 0) {
		print::err(
			"'unsafe_decrease_reference_count_without_destroing'"
			" on object without references\n"
		);
		posix::abort();
	}
	--references_;
}

inline reference create_object(_class& c) {
	c.initialise_if_need();
	object* ptr = posix::allocate_raw_memory_of<object>(1).iterator();
	new(ptr) object(c);
	return { *ptr };
}
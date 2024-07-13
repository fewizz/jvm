#include "decl/object.hpp"

#include "decl/mutex_attribute_recursive.hpp"
#include "decl/class.hpp"
#include "decl/array.hpp"

#include "execution/info.hpp"

#include <posix/memory.hpp>
#include <posix/io.hpp>

inline object::object(::c& c) :
	class_{ c },
	mutex_{ posix::create_mutex(mutex_attribute_recursive) },
	data_ { posix::allocate<>(c.instance_layout().size()) }
{
	if(info) {
		tabs();
		print::out("# constructing object @");
		print::out.hex((nuint)this);
		auto name = c.name();
		print::out(" of type ", name, "\n");
	}

	for (instance_field_index field_index : c.instance_fields().index_view()) {
		view_ptr(field_index, []<typename Type>(Type* ptr) {
			new (ptr) Type();
		});
	}
}

inline object::~object() {
	if(info) {
		tabs();
		auto name = c().name();
		print::out("# destructing object @").hex((nuint)this);
		print::out(" of type ", name, "\n");
	}

	if(c().is_array()) {
		uint8* data = array_data<uint8>(*this);
		if(c().get_component_class().is_not_primitive()) {
			for(nuint x = array_length(*this); x > 0; --x) {
				((reference*) data)[x - 1].~reference();
			}
		}
		posix::free_raw_memory(data);
	}

	for (instance_field_index field_index : class_.instance_fields().index_view()) {
		view(field_index, []<typename Type>(Type& e) {
			e.~Type();
		});
	}
}

inline void object::on_reference_added() {
	++references_;
	if(info) {
		tabs();
		print::out("# added reference to object @");
		print::out.hex((nuint)this);
		auto name = c().name();
		print::out(" of type ", name, ", ", references_, " in sum\n");
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

inline void object::on_reference_removed() {
	if(references_ == 0) {
		print::err("# removing reference on object without references\n");
		posix::abort();
	}
	--references_;
	if(info) {
		tabs();
		print::out("# removed reference to object @").hex((nuint)this);
		auto name = c().name();
		print::out(" of type ", name, ", ", references_, " left\n");
	}
	if(references_ == 0) {
		uint8* ptr_to_this = (uint8*) this;

		(*this).~object();

		posix::free_raw_memory(ptr_to_this);
	}
}

[[nodiscard]] inline expected<reference, reference>
try_create_object(c& c) {
	optional<reference> possible_throwable = c.try_initialise_if_need();
	if(possible_throwable.has_value()) {
		return unexpected{ possible_throwable.move() };
	}

	object* ptr = posix::allocate_raw<object>(1).iterator();

	new(ptr) object(c);

	return { *ptr };
}
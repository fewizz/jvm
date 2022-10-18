#include "decl/object.hpp"

#include "class.hpp"
#include "field.hpp"
#include "array.hpp"

#include "execution/info.hpp"

#include <posix/memory.hpp>
#include <posix/io.hpp>

inline object::object(::_class& c) :
	class_{ c },
	values_ {
		posix::allocate_memory_for<field_value>(c.instance_fields().size())
	}
{
	for(field* instance_field : c.instance_fields()) {
		values_.emplace_back(instance_field->descriptor());
	}

	if(info) {
		tabs();
		print("constructing object @");
		print_hex((nuint)this);
		print(" of type ");
		auto name = _class().name();
		print(name);
		print("\n");
	}
}

inline object::~object() {
	if(_class().is_array()) {
		uint8* data = array_data<uint8>(*this);

		if(!_class().get_component_class().is_primitive()) {
			for(nuint x = array_length(*this); x > 0; --x) {
				((reference*) data)[x - 1].~reference();
			}
		}
		// TODO compute actual size, uses free so its safe
		posix::free_raw_memory(data);
	}
	if(info) {
		tabs();
		print("destructing object @");
		print_hex((nuint)this);
		print(" of type ");
		auto name = _class().name();
		print(name);
		print("\n");
	}
}

inline void object::on_reference_added() {
	++references_;
	if(info) {
		tabs();
		print("added reference to object @");
		print_hex((nuint)this);
		print(", now there is ");
		print(references_);
		print("\n");
	}
}

inline void object::on_reference_removed() {
	if(references_ == 0) {
		posix::std_err.write_from(
			c_string{"removing reference on object without references\n"}
		);
		posix::abort();
	}
	--references_;
	if(info) {
		tabs();
		print("removed reference to object @");
		print_hex((nuint)this);
		print(", now there is ");
		print(references_);
		print("\n");
	}
	if(references_ == 0) {
		uint8* ptr_to_this = (uint8*) this;
		this->~object();
		posix::free_raw_memory(ptr_to_this);
	}
}

inline void object::unsafe_decrease_reference_count_without_destroing() {
	if(references_ == 0) {
		posix::std_err.write_from(c_string{
			"'unsafe_decrease_reference_count_without_destroing'"
			" on object without references\n"
		});
		posix::abort();
	}
	--references_;
}

inline field_value& object::operator [] (instance_field_index index) {
	return values_[(uint16) index];
}
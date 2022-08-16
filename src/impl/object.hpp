#include "decl/object.hpp"

#include "class.hpp"
#include "field.hpp"
#include "array.hpp"
#include "abort.hpp"
#include "range.hpp"

#include "execution/info.hpp"

inline object::object(::_class& c) :
	class_{ c },
	values_{ allocate_for<field_value>(c.instance_fields().size()) }
{
	for(field& instance_field : c.instance_fields()) {
		values_.emplace_back(instance_field.descriptor());
	}

	if(info) {
		tabs();
		fprintf(
			stderr,
			"object constructed with address = %p, type = ", this
		);
		auto name = _class().name();
		fwrite(name.elements_ptr(), 1, name.size(), stderr);
		fputc('\n', stderr);
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
		deallocate(memory_span{ data, 0 });
	}
	if(info) {
		tabs();
		fprintf(
			stderr,
			"object destructed with address = %p\n", this
		);
	}
}

inline void object::on_reference_added() {
	tabs();
		fprintf(
		stderr,
		"on reference added for object with address = %p\n", this
	);
	++references_;
}

inline void object::on_reference_removed() {
	tabs();
		fprintf(
		stderr,
		"on reference removed for object with address = %p\n", this
	);
	if(references_ == 0) {
		fputs("'on_reference_removed' on object without references", stderr);
		abort();
	}
	--references_;
	if(references_ == 0) {
		uint8* ptr_to_this = (uint8*) this;
		this->~object();
		deallocate(memory_span{ ptr_to_this, sizeof(object) });
	}
}

inline void object::unsafe_decrease_reference_count_without_destroing() {
	if(references_ == 0) {
		fputs(
			"'unsafe_decrease_reference_count_without_destroing'"
			" on object without references",
			stderr
		);
		abort();
	}
	--references_;
}

inline field_value& object::operator [] (instance_field_index index) {
	return values_[(uint16) index];
}
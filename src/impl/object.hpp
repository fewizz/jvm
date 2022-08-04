#include "decl/object.hpp"

#include "class.hpp"
#include "field.hpp"
#include "array.hpp"

#include "execution/info.hpp"

inline object::object(::_class& c) :
	class_{ c },
	values_{ c.instance_fields_count() }
{
	c.for_each_instance_field([&](instance_field_with_class fwc) {
		using namespace class_file;

		field_value fv;

		bool result = descriptor::read_field(
			fwc.descriptor().begin(),
			[&]<typename DescriptorType>(DescriptorType) {
				return fv.set_default_value<DescriptorType>();
			}
		);

		if(!result) {
			fputs(
				"couldn't read field descriptor while creating object",
				stderr
			);
			abort();
		}

		values_.emplace_back(move(fv));
	});

	if(info) {
		tabs();
		fprintf(
			stderr,
			"object constructed with address = %p, type = ", this
		);
		auto name = _class().name();
		fwrite(name.data(), 1, name.size(), stderr);
		fputc('\n', stderr);
	}
}

inline object::~object() {
	if(_class().is_array_class()) {
		uint8* data = array_data<uint8>(*this);

		if(!_class().get_component_class().is_primitive_class()) {
			for(nuint x = array_length(*this); x > 0; --x) {
				((reference*) data)[x - 1].~reference();
			}
		}
		default_allocator{}.deallocate(
			data, 0 // TODO compute actual size, uses free so its safe
		);
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
	/*tabs();
		fprintf(
		stderr,
		"on reference added for object with address = %p\n", this
	);*/
	++references_;
}

inline void object::on_reference_removed() {
	/*tabs();
		fprintf(
		stderr,
		"on reference removed for object with address = %p\n", this
	);*/
	if(references_ == 0) {
		fputs("'on_reference_removed' on object without references", stderr);
		abort();
	}
	--references_;
	if(references_ == 0) {
		uint8* ptr_to_this = (uint8*) this;
		this->~object();
		default_allocator{}.deallocate(ptr_to_this, sizeof(object));
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
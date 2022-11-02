#pragma once

#include "../field.hpp"
#include "../object/reference.hpp"

#include <posix/memory.hpp>
#include <range.hpp>
#include <max.hpp>

struct layout_offset {
private:
	nuint value_;
public:
	layout_offset(nuint value) : value_{ value } {}
	explicit operator nuint () { return value_; }
};

struct layout {

	posix::memory_for_range_of<layout_offset> field_index_to_offset{};
	nuint max_field_boundary = 0;

	layout() = default;

	void init(layout& super, basic_range auto& fields) {
		field_index_to_offset =
			posix::allocate_memory_for<nuint>(fields.size());
		
		super.field_index_to_offset.as_span()
			.copy_to(this->field_index_to_offset.as_span());
		nuint last_offset_index = super.field_index_to_offset.size();
		init(
			fields,
			last_offset_index + 1,
			super.max_field_boundary
		);
	}

	void init(auto& fields) {
		field_index_to_offset =
			posix::allocate_memory_for<nuint>(fields.size());
		init(fields, 0, 0);
	}

private:
	void init(
		auto& fields,
		nuint initial_field_index,
		nuint initial_field_offset
	) {
		field_index_to_offset =
			posix::allocate_memory_for<nuint>(fields.size());
		nuint current_offset = initial_field_offset;

		auto align = [&](nuint alignment) {
			nuint o = current_offset & (alignment - 1);
			if(o > 0) {
				current_offset += alignment - o;
			}
		};

		auto add = [&]<typename Type, typename... Types>() {
			nuint current_field_index = initial_field_index;

			while(current_field_index < fields.size()) {
				field& f = fields[current_field_index];
				if((f.type.is<Types>() || ... )) {
					align(alignof(Type));
					field_index_to_offset[current_field_index]
						.construct(current_offset);
					current_offset += sizeof(Type);
					max_field_boundary =
						max(max_field_boundary, current_offset);
				}
				current_field_index++;
			}
		};

		add.template operator () <
			reference, class_file::object, class_file::array
		>();
		add.template operator () <int64,  class_file::j>();
		add.template operator () <double, class_file::d>();
		add.template operator () <int32,  class_file::i>();
		add.template operator () <float,  class_file::f>();
		add.template operator () <uint16, class_file::c>();
		add.template operator () <int16,  class_file::s>();
		add.template operator () <int8,   class_file::b>();
		add.template operator () <bool,   class_file::z>();
	}

};
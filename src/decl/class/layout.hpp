#pragma once

#include "./instance_field_position.hpp"
#include "../field.hpp"
#include "../reference.hpp"

#include <posix/memory.hpp>
#include <range.hpp>
#include <max.hpp>

struct layout {

	struct slot {
	private:
		nuint beginning_;
		nuint length_;
	public:
		slot(nuint beginning, nuint length) :
			beginning_{ beginning },
			length_{ length }
		{}

		instance_field_position position() const {
			return instance_field_position{ beginning() };
		};
		nuint beginning() const { return beginning_; }
		nuint length() const { return length_; }
		nuint ending() const { return beginning() + length(); }
	};

	nuint ending_ = 0;
	posix::memory_for_range_of<slot> field_index_to_slot_{};

	layout(layout&&) = default;

	template<basic_range Range>
	layout(
		Range&& declared_instance_fields,
		optional<const layout&> super = {}
	) :
		field_index_to_slot_ { [&] {
			nuint count = 0;
			if(super.has_value()) {
				count += range_size(super->field_index_to_slot_);
			}
			count += range_size(declared_instance_fields);

			auto field_index_to_slot = posix::allocate_memory_for<slot>(count);
			nuint current_position = 0;
			nuint initial_field_index = 0;
			if(super.has_value()) {
				super->field_index_to_slot_.as_span().for_each_indexed(
					[&](slot s, nuint index) {
						field_index_to_slot[index].construct(s);
					}
				);
				current_position = super->ending_;
				initial_field_index = range_size(super->field_index_to_slot_);
			}

			auto align = [&](nuint alignment_bytes) {
				nuint o = current_position % alignment_bytes;
				if(o > 0) {
					current_position += alignment_bytes - o;
				}
			};

			auto add = [&]<typename Type, typename... Types>() {
				nuint field_index = initial_field_index;
				for(field& f : declared_instance_fields) {
					if((f.type.is_same_as<Types>() || ... )) {
						align(bytes_in_atoms<alignof(Type)>);
						field_index_to_slot[field_index].construct(
							current_position, bytes_in<Type>
						);
						current_position += bytes_in<Type>;
					}
					++field_index;
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

			ending_ = current_position;

			return move(field_index_to_slot);
		}()}
	{}

	nuint total_size() const {
		return ending_;
	}

	slot slot_for_field_index(nuint index) const {
		return field_index_to_slot_.as_span()[index];
	}

};
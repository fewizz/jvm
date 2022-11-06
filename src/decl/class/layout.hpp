#pragma once

#include "./instance_field_position.hpp"
#include "../field.hpp"
#include "../reference.hpp"

#include <posix/memory.hpp>
#include <range.hpp>
#include <max.hpp>
#include <list.hpp>

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

	nuint ending = 0;
	posix::memory_for_range_of<slot> field_index_to_slot{};

	layout(layout&&) = default;

	template<basic_range Range>
	layout(Range&& instance_fields, optional<layout&> super = {}) :
		field_index_to_slot { [&] {
			nuint count = range_size(instance_fields);
			if(super.has_value()) {
				count += range_size(super.value().field_index_to_slot);
			}

			list field_index_to_slot{ posix::allocate_memory_for<slot>(count) };
			nuint current_position = 0;
			if(super.has_value()) {
				field_index_to_slot.put_back_copied_elements_of(
					super->field_index_to_slot.as_span()
				);
				current_position = super->ending;
			}

			nuint initial_field_index = field_index_to_slot.size();

			auto align = [&](nuint alignment_bytes) {
				nuint o = current_position % alignment_bytes;
				if(o > 0) {
					current_position += alignment_bytes - o;
				}
			};

			auto add = [&]<typename Type, typename... Types>() {
				nuint current_field_index = initial_field_index;

				while(current_field_index < range_size(instance_fields)) {
					field& f = instance_fields[current_field_index];
					if((f.type.is<Types>() || ... )) {
						align(bytes_in_atoms<alignof(Type)>);
						field_index_to_slot.emplace_back(
							current_position, bytes_in<Type>
						);
						current_position += bytes_in<Type>;
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

			ending = current_position;

			return field_index_to_slot.move_storage_range();
		}()}
	{}

	nuint total_size() const {
		return ending;
	}

	slot slot_for_field_index(nuint index) {
		return field_index_to_slot.as_span()[index];
	}

};
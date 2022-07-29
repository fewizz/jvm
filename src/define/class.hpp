#pragma once

#include "./read_method.hpp"

#include "class/es/find_or_load.hpp"
#include "field.hpp"
#include "method.hpp"
#include "execute.hpp"
#include "abort.hpp"

#include <class_file/reader.hpp>
#include <class_file/descriptor/reader.hpp>

#include <core/span.hpp>
#include <core/c_string.hpp>
#include <core/concat.hpp>
#include <core/range_of_value_type_same_as.hpp>
#include <core/meta/types/are_exclusively_satisfying_predicates.hpp>
#include <core/range_of_value_type_same_as.hpp>
#include <core/meta/decayed_satisfying_predicate.hpp>
#include <stdio.h>

template<range_of<uint8> BytesRange>
static inline _class& define_class(BytesRange&& bytes) {
	using namespace class_file;

	reader reader{ bytes.data() };

	auto [magic_exists, version_reader] =
		reader.check_for_magic_and_get_version_reader();

	if(!magic_exists) {
		fprintf(stderr, "magic doesn't exist");
		abort();
	}

	auto [version, constant_pool_reader] =
		version_reader.read_and_get_constant_pool_reader();

	uint16 constants_count = constant_pool_reader.entries_count();
	const_pool const_pool{ constants_count };

	auto access_flags_reader {
		constant_pool_reader.read_and_get_access_flags_reader(
			[&]<typename Type>(Type x) {
				if constexpr(same_as<constant::unknown, Type>) {
					fprintf(stderr, "unknown constant with tag %hhu", x.tag);
					abort();
				}
				else {
					const_pool.emplace_back(x);
				}
			}
		)
	};

	auto [access_flags, this_class_reader] {
		access_flags_reader.read_and_get_this_class_reader()
	};
	auto [this_class, super_class_reader] {
		this_class_reader.read_and_get_super_class_reader()
	};
	auto [super_class, interfaces_reader] {
		super_class_reader.read_and_get_interfaces_reader()
	};

	interfaces_indices_container interfaces{ interfaces_reader.count() };

	auto fields_reader {
		interfaces_reader.read_and_get_fields_reader(
			[&](class_file::interface_index interface_index) {
				interfaces.emplace_back(interface_index);
			}
		)
	};

	uint16 fields_count = fields_reader.count();

	limited_list<
		::field, uint16, default_allocator
	> fields{ fields_count };

	uint16 instance_fields_count = 0;
	uint16 static_fields_count = 0;

	auto methods_reader = fields_reader.read_and_get_methods_reader(
		[&](auto field_reader) {
			auto [access_flags, name_index_reader] {
				field_reader.read_access_flags_and_get_name_index_reader()
			};
			auto [name_index, descriptor_index_reader] {
				name_index_reader.read_and_get_descriptor_index_reader()
			};
			auto [descriptor_index, attributes_reader] {
				descriptor_index_reader.read_and_get_attributes_reader()
			};
			auto it = attributes_reader.read_and_get_advanced_iterator(
				[&](auto name_index) {
					return const_pool.utf8_constant(name_index);
				},
				[&]<typename Type>(Type) {
				}
			);

			if(access_flags._static()) {
				++static_fields_count;
			}
			else {
				++instance_fields_count;
			}
			fields.emplace_back(
				access_flags,
				::name_index{ name_index },
				::descriptor_index{ descriptor_index }
			);
			return it;
		}
	);

	instance_fields_container instance_fields{ instance_fields_count };
	static_fields_container static_fields{ static_fields_count };

	for(::field& field : fields) {
		if(field.is_static()) {
			static_fields.emplace_back(move(field), const_pool);
		}
		else {
			instance_fields.emplace_back(move(field));
		}
	}

	methods_container methods{ methods_reader.count() };

	methods_reader.read_and_get_attributes_reader([&](auto method_reader) {
		auto [m, it] = read_method_and_get_advaned_iterator(
			const_pool, method_reader
		);
		methods.emplace_back(move(m));
		return it;
	});

	return classes.emplace_back(
		move(const_pool),
		span<uint8>{ bytes.data(), bytes.size() }, access_flags,
		this_class_index{ this_class }, super_class_index{ super_class },
		move(interfaces),
		move(instance_fields),
		move(static_fields),
		move(methods),
		is_array_class{ false },
		is_primitive_class{ false }
	);
}
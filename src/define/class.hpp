#pragma once

#include "./fields_to_add.hpp"
#include "./constants_to_add.hpp"
#include "./read_method.hpp"

#include "classes/find_or_load.hpp"
#include "field/decl.hpp"
#include "method/decl.hpp"
#include "execute/decl.hpp"
#include "abort.hpp"

#include <class/file/reader.hpp>
#include <class/file/descriptor/reader.hpp>

#include <core/span.hpp>
#include <core/c_string.hpp>
#include <core/concat.hpp>
#include <core/meta/types/are_exclusively_satisfying_predicates.hpp>
#include <core/range_of_value_type_same_as.hpp>
#include <core/meta/decayed_satisfying_predicate.hpp>
#include <stdio.h>

template<typename... Args>
static inline _class& define_class0(Args&&... args) {
	using namespace class_file;

	auto& bytes = elements::range_of<uint8>(args...);

	reader magic_reader{ bytes.data() };
	auto [version_reader, magic_exists] = magic_reader();
	if(!magic_exists) {
		fprintf(stderr, "magic doesn't exist");
		abort();
	}

	auto [read_constant_pool, version] = version_reader();

	uint16 constants_count = read_constant_pool.entries_count();
	const_pool const_pool{ constants_count };

	auto read_access_flags = read_constant_pool([&]<typename Type>(Type x) {
		if constexpr(same_as<constant::unknown, Type>) {
			fprintf(stderr, "unknown constant with tag %hhu", x.tag);
			abort();
		}
		else {
			const_pool.emplace_back(x);
		}
	});

	auto [read_this_class, access_flags] = read_access_flags();
	auto [read_super_class, this_class] = read_this_class();
	auto [read_interfaces, super_class] = read_super_class();

	interfaces_indices_container interfaces{ read_interfaces.count() };

	auto read_fields = read_interfaces([&](uint16 interface_index) {
		interfaces.emplace_back(interface_index);
	});

	uint16 fields_count = read_fields.count();

	limited_list<
		::field, uint16, default_allocator
	> fields{ fields_count };

	uint16 instance_fields_count = 0;
	uint16 static_fields_count = 0;

	auto methods_reader = read_fields([&](auto field_reader) {
		auto access_reader = field_reader;
		auto [name_index_reader, access_flags] = access_reader();
		auto [descriptor_reader, name_index] = name_index_reader();
		auto [attributes_reader, descriptor_index] = descriptor_reader();
		auto end = attributes_reader(
			[&](auto name_index) {
				return const_pool.utf8_constant(name_index);
			},
			[&]<typename Type>(Type) {

			}
		);
		bool is_static = access_flags.get(class_file::access_flag::_static);
		if(is_static) {
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
		return end;
	});

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

	methods_reader([&](auto method_reader) {
		auto [reader, m] = read_method(const_pool, method_reader);
		methods.emplace_back(move(m));
		return reader;
	});

	classes.emplace_back(
		move(const_pool),
		span<uint8>{ bytes.data(), bytes.size() }, access_flags,
		this_class_index{ this_class }, super_class_index{ super_class },
		move(interfaces),
		move(instance_fields),
		move(static_fields),
		move(methods)
	);

	return classes.back();
}

template<typename... Args>
requires types::are_exclusively_satisfying_predicates<
	types::are_contain_range_of<uint8>,
	types::are_may_contain_one_satisfying_predicate<is_fields_to_add>,
	types::are_may_contain_one_satisfying_predicate<is_constants_to_add>
>::for_types<Args...>
inline _class& define_class(Args&&... args) {
	return define_class0(forward<Args>(args)...);
}
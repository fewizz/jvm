#pragma once

#include "instance_fields_to_add.hpp"
#include "read_field.hpp"
#include "read_method.hpp"

#include "../classes.hpp"
#include "../field.hpp"
#include "../method.hpp"
#include "../execute.hpp"
#include "class/file/reader.hpp"
#include "class/file/descriptor/reader.hpp"
#include "../../abort.hpp"

#include <core/span.hpp>
#include <core/c_string.hpp>
#include <core/concat.hpp>
#include <core/meta/types/are_exclusively_satisfying_predicates.hpp>
#include <core/range_of_value_type_same_as.hpp>
#include <core/meta/decayed_satisfying_predicate.hpp>
#include <stdio.h>

template<range Name>
_class& find_or_load(Name name);

template<typename... Args>
inline _class& define_class0(Args&&... args) {
	using namespace class_file;

	auto& bytes = elements::range_of<uint8>(args...);

	reader magic_reader{ bytes.data() };
	auto [version_reader, magic_exists] = magic_reader();
	if(!magic_exists) {
		fprintf(stderr, "magic doesn't exist");
		abort();
	}

	auto [read_constant_pool, version] = version_reader();

	classes.emplace_back(const_pool{ read_constant_pool.entries_count() });

	_class& c = classes.back();
	c.data_ = { bytes };

	auto read_access_flags = read_constant_pool(
		[&]<typename Type>(Type x, uint16) {
			if constexpr(same_as<constant::unknown, Type>) {
				fprintf(stderr, "unknown constant with tag %hhu", x.tag);
				abort();
			}
			else {
				c.const_pool::emplace_back(x);
				c.trampoline_pool::emplace_back(nullptr);
			}
		}
	);

	auto [read_this_class, access_flags] = read_access_flags();
	auto [read_super_class, this_class] = read_this_class();
	auto [read_interfaces, super_class] = read_super_class();
	c.access_flags_ = access_flags;
	c.this_class_index_ = name_index{ this_class };
	c.super_class_index_ = name_index{ super_class };

	c.interfaces_ = { read_interfaces.count()};

	auto read_fields = read_interfaces([&](uint16 interface_index) {
		c.interfaces_.emplace_back(interface_index);
	});

	uint16 fields_count = read_fields.count();

	if constexpr(
		types::are_contain_decayed_satisfying_predicate<
			is_instance_fields_to_add
		>::for_types<Args...>
	) {
		auto& to_add = elements::decayed_satisfying_predicate<
			is_instance_fields_to_add
		>(args...);
		fields_count += to_add.fields().size();
	}

	c.fields_ = { fields_count };
	auto methods_reader = read_fields([&](auto field_reader) {
		auto [reader, f] = read_field(c, field_reader);
		c.fields_.emplace_back(move(f));
		return reader;
	});

	if constexpr(
		types::are_contain_decayed_satisfying_predicate<
			is_instance_fields_to_add
		>::for_types<Args...>
	) {
		auto& to_add = elements::decayed_satisfying_predicate<
			is_instance_fields_to_add
		>(args...);
		for(field_constructor_args& f : to_add.fields()) {
			f.pass([&](auto... args) {
				c.fields_.emplace_back(::field { c, args... });
			});
		}
	}

	c.methods_ = { methods_reader.count() };

	methods_reader([&](auto method_reader) {
		auto [reader, m] = read_method(c, method_reader);
		c.methods_.emplace_back(move(m));
		return reader;
	});

	if(c.super_class_index() != 0) {
		_class& s = find_or_load(
			c.utf8_constant(c.class_constant(c.super_class_index()).name_index)
		);
		uint16 instance_fields = s.instance_fields_.size();
		for(auto& f : c.fields_) {
			if(!f.get<::field>().is_static()) ++instance_fields;
		}

		c.instance_fields_ = { instance_fields };

		for(auto& f : s.fields_) {
			c.instance_fields_.emplace_back(&f.get<::field>());
		}
	}

	for(auto& f : c.fields_) {
		if(!f.get<::field>().is_static()) {
			c.instance_fields_.emplace_back(&f.get<::field>());
		}
	}

	for(auto interface_index : c.interfaces_indices()) {
		c.get_class(interface_index);
	}

	if(
		auto clinit = c.try_find_method(c_string{ "<clinit>" });
		clinit != nullptr
	) {
		execute(*clinit);
	}

	return c;
}

template<typename... Args>
requires types::are_exclusively_satisfying_predicates<
	types::are_contain_range_of<uint8>,
	types::are_may_contain_one_satisfying_predicate<is_instance_fields_to_add>
>::for_types<Args...>
inline _class& define_class(Args&&... args) {
	return define_class0(forward<Args>(args)...);
}
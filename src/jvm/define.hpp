#pragma once

#include "classes.hpp"
#include "field.hpp"
#include "method.hpp"
#include "class/file/reader.hpp"
#include "class/file/descriptor/reader.hpp"
#include "../abort.hpp"

#include <core/span.hpp>
#include <stdio.h>

template<typename Iterator>
elements::of<
	class_file::field::reader<Iterator, class_file::field::reader_stage::end>,
	elements::one_of<field, static_field>
>
read_field(_class& c, class_file::field::reader<Iterator> access_reader) {
	auto [name_index_reader, access_flags] = access_reader();
	auto [descriptor_reader, name_index] = name_index_reader();
	auto [attributes_reader, descriptor_index] = descriptor_reader();
	auto end = attributes_reader(
		[&](auto name_index){ return c.utf8_constant(name_index); },
		[&]<typename Type>(Type) {
		}
	);
	field f{ c, access_flags, name_index, descriptor_index };
	if(!access_flags.get(class_file::access_flag::_static))
		return { end, f };

	field_value fv;

	auto descriptor = c.utf8_constant(descriptor_index);
	using namespace class_file;
	bool result = descriptor::read_field(
		descriptor.begin(),
		[&]<typename Type0>(Type0) {
			if constexpr(same_as<descriptor::B, Type0>) {
				fv = jbyte{ 0 };
			} else
			if constexpr(same_as<descriptor::C, Type0>) {
				fv = jbyte{ 0 };
			} else
			if constexpr(same_as<descriptor::D, Type0>) {
				fv = jdouble{ 0.0 };
			} else
			if constexpr(same_as<descriptor::F, Type0>) {
				fv = jfloat{ 0.0 };
			} else
			if constexpr(same_as<descriptor::I, Type0>) {
				fv = jint{ 0 };
			} else
			if constexpr(same_as<descriptor::J, Type0>) {
				fv = jlong{ 0 };
			} else
			if constexpr(same_as<descriptor::Z, Type0>) {
				fv = jbool{ 0 };
			} else {
				return false;
			}
				return true;
		}
	);
	if(!result) {
		fputs("couldn't read field descriptor", stderr);
		abort();
	}
	return { end, { f, fv } };
}

template<typename Iterator>
elements::of<
	class_file::method::reader<Iterator, class_file::method::reader_stage::end>,
	method
>
read_method(_class& c, class_file::method::reader<Iterator> read_access) {
	auto [read_name_index, access_flags] = read_access();
	auto [descriptor_reader, name_index] = read_name_index();
	auto [read_attributes, descriptor_index] = descriptor_reader();

	code code;

	auto end = read_attributes(
		[&](auto name_index){ return c.utf8_constant(name_index); },
		[&]<typename Type>(Type x) {
			if constexpr (Type::type == class_file::attribute::type::code) {
				auto [read_max_locals, max_stack] = x();
				auto [read_code, max_locals] = read_max_locals();
				auto src0 = read_code.iterator_;
				uint32 length = read<uint32, endianness::big>(src0);
				code = ::code {
					span<uint8, uint32> { read_code.iterator_, length },
					max_stack, max_locals
				};
			}
		}
	);
	return { end, { c, access_flags, name_index, descriptor_index, code } };
}

inline _class& define_class(span<uint8> bytes) {
	using namespace class_file;

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
	c.this_class_index_ = this_class;
	c.super_class_index_ = super_class;

	c.interfaces_ = { read_interfaces.count()};

	auto read_fields = read_interfaces([&](uint16 interface_index) {
		c.interfaces_.emplace_back(interface_index);
	});

	c.fields_ = { read_fields.count() };
	auto methods_reader = read_fields([&](auto field_reader) {
		auto [reader, f] = read_field(c, field_reader);
		c.fields_.emplace_back(move(f));
		return reader;
	});

	c.methods_ = { methods_reader.count() };

	methods_reader([&](auto method_reader) {
		auto [reader, m] = read_method(c, method_reader);
		c.methods_.emplace_back(move(m));
		return reader;
	});

	if(c.super_class_index() != 0) {
		_class& s = load_class(
			c.utf8_constant(
				c.class_constant(c.super_class_index()).name_index)
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

	return c;
}

#include <core/concat.hpp>
#include <core/copy.hpp>
#include <core/c_string.hpp>
#include "../alloc.hpp"

inline _class& define_array_class(_class& element_class) {
	classes.emplace_back(const_pool{ 6 });
	_class& c = classes.back();

	auto name = concat_view{ element_class.name(), array{ '[', ']' } };
	c_string ptr_name { "ptr_" };
	c_string ptr_desc { "J" };
	c_string len_name { "len_" };
	c_string len_desc { "I" };
	auto concated = concat_view(name, ptr_name, ptr_desc, len_name, len_desc);
	auto concated_size = concated.size();

	c.data_ = { default_allocator{}.allocate(concated_size), concated_size };
	copy{ concated }.to(c.data_);

	auto ptr = (char*) c.data_.data();
	c.const_pool::emplace_back(
		class_file::constant::utf8 { ptr, (uint16) name.size() }
	);
	c.const_pool::emplace_back(class_file::constant::_class { 0 });
	ptr += name.size();

	c.const_pool::emplace_back(
		class_file::constant::utf8 { ptr, (uint16) ptr_name.size() }
	);
	ptr += ptr_name.size();

	c.const_pool::emplace_back(
		class_file::constant::utf8 { ptr, (uint16) ptr_desc.size() }
	);
	ptr += ptr_desc.size();

	c.const_pool::emplace_back(
		class_file::constant::utf8 { ptr, (uint16) len_name.size() }
	);
	ptr += len_name.size();

	c.const_pool::emplace_back(
		class_file::constant::utf8 { ptr, (uint16) len_desc.size() }
	);
	//ptr += len_desc.size();

	c.fields_ = { 2 };
	c.fields_.emplace_back(
		field {
			c,
			class_file::access_flags{ class_file::access_flag::_private },
			3, 4
		}
	);
	c.fields_.emplace_back(
		field {
			c,
			class_file::access_flags{ class_file::access_flag::_private },
			5, 6
		}
	);

	c.instance_fields_ = { 2 };
	c.instance_fields_.emplace_back(&c.fields_[0].get<field>());
	c.instance_fields_.emplace_back(&c.fields_[1].get<field>());

	c.this_class_index_ = 2;

	return c;
}

template<range Name>
inline _class& define_primitive_class(Name&& name) {
	classes.emplace_back(const_pool{ 2 });
	_class& c = classes.back();

	c.data_ = {
		default_allocator{}.allocate(name.size()), (uint16) name.size()
	};
	copy{ name }.to(c.data_);

	c.const_pool::emplace_back(
		class_file::constant::utf8 {
			(char*) c.data_.data(), (uint16) name.size()
		}
	);
	c.const_pool::emplace_back(class_file::constant::_class{ 1 });

	c.this_class_index_ = 2;

	return c;
}
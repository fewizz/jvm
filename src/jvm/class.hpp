#pragma once

#include "const_pool.hpp"

#include "object.hpp"
#include "../abort.hpp"
#include "class/file/access_flag.hpp"

#include <core/range.hpp>
#include <core/fixed_vector.hpp>

#include <stdio.h>

struct field;
struct method;

struct _class : const_pool {
private:
	class_file::access_flags access_flags_;
	uint16 this_class_index_;
	uint16 super_class_index_;
	::fixed_vector<uint16, uint16, default_allocator> interfaces_;
	::fixed_vector<field, uint16, default_allocator> fields_;
	::fixed_vector<method, uint16, default_allocator> methods_;
	::fixed_vector<const void*, uint16, default_allocator> trampolines_;
	::fixed_vector<field_value, uint16, default_allocator> static_values_;
	uint8* data_;

	friend inline _class& define_class(span<uint8> bytes);

public:

	_class(const _class&) = delete;
	_class(_class&&) = default;

	inline _class(const_pool&& const_pool);
	inline ~_class();

	auto name() const {
		return utf8_entry(class_entry(this_class_index_).name_index);
	}

	template<typename Type>
	Type* trampoline(uint16 index) {
		return (Type*) trampolines_[index - 1];
	}

	void trampoline(uint16 index, auto& ref) {
		trampolines_[index - 1] = &ref;
	}

	template<range Name>
	inline const method* try_find_method(Name name);

	template<range Name>
	inline const field* try_find_field(Name name);

	template<range Name>
	const method& find_method(Name&& name) {
		if(auto m = try_find_method(name); m != nullptr) {
			return *m;
		}
		fprintf(stderr, "couldn't find method %s", name.data());
		abort();
	}

	template<range Name>
	const field& find_field(Name&& name) {
		if(auto f = try_find_field(name); f != nullptr) {
			return *f;
		}
		fprintf(stderr, "couldn't find field %s", name.data());
		abort();
	}

	inline const method& get_or_load_method_by_ref_index(
		uint16 ref_index
	);

	inline const field& get_or_load_field_by_ref_index(
		uint16 ref_index
	);

	inline const field_value& get_or_load_static_field_value_by_ref(
		uint16 ref_index
	);
};

#include "field.hpp"
#include "method.hpp"

_class::_class(const_pool&& const_pool) :
	::const_pool{ move(const_pool) },
	trampolines_{ entries_count() },
	static_values_{ entries_count() }
{}

_class::~_class() { free(data_); }

template<range Name>
const field* _class::try_find_field(Name name) {
	for(auto& f : fields_) if(equals(f.name(), name)) return &f;
	return nullptr;
}

template<range Name>
const method* _class::try_find_method(Name name) {
	for(auto& m : methods_) if(equals(m.name(), name)) return &m;
	return nullptr;
}

#include "classes.hpp"
#include "load.hpp"
#include "class/file/descriptor/reader.hpp"

#include <core/equals.hpp>
#include <core/to_range.hpp>

const method&
_class::get_or_load_method_by_ref_index(uint16 ref_index) {
	if(auto t = trampoline<method>(ref_index); t != nullptr) {
		return *t;
	}

	auto ref = method_ref_entry(ref_index);
	auto class_info = class_entry(ref.class_index);
	auto nat = name_and_type_entry(ref.name_and_type_index);
	auto name = utf8_entry(nat.name_index);
	auto descriptor = utf8_entry(nat.descriptor_index);

	class_file::descriptor::method_reader params{ descriptor.begin() };
	auto [ret_reader, result0] = params([&]<typename Type>(Type x) {
		if constexpr(same_as<Type, class_file::descriptor::object_type>) {
			find_or_load(x);
		}
		return true;
	});
	if(!result0) {
		fprintf(stderr, "couldn't read method descriptor parameters");
		abort();
	}

	auto [end, result1] = ret_reader([&]<typename Type>(Type x) {
		if constexpr(same_as<Type, class_file::descriptor::object_type>) {
			find_or_load(x);
		}
		return true;
	});

	if(!result1) {
		fprintf(stderr, "couldn't read method descriptor return type");
		abort();
	}

	auto class_name = utf8_entry(class_info.name_index);
	_class& c = find_or_load(class_name);

	const method& m = c.find_method(name);
	trampoline(ref_index, m);
	return m;
}

const field&
_class::get_or_load_field_by_ref_index(uint16 ref_index) {
	if(auto t = trampoline<field>(ref_index); t != nullptr) {
		return *t;
	}

	using namespace class_file;

	constant::field_ref ref = field_ref_entry(ref_index);
	constant::_class class_info = class_entry(ref.class_index);
	constant::name_and_type nat = name_and_type_entry(ref.name_and_type_index);
	constant::utf8 name = utf8_entry(nat.name_index);
	constant::utf8 descriptor = utf8_entry(nat.descriptor_index);

	bool result = class_file::descriptor::read_field(
		descriptor.begin(),
		[&]<typename Type>(Type x) {
			if constexpr(same_as<Type, class_file::descriptor::object_type>) {
				find_or_load(x);
			}
			return true;
		}
	);

	if(!result) {
		fprintf(stderr, "couldn't read field descriptor");
		abort();
	}

	auto clss = utf8_entry(class_info.name_index);
	_class& c = find_or_load(clss);

	const ::field& f = c.find_field(name);
	trampoline(ref_index, f);
	return f;
}

const field_value&
_class::get_or_load_static_field_value_by_ref(uint16 ref_index) {
	/*if(auto t = trampoline<field>(ref_index); t != nullptr) {
		auto& val = static_values_[ref_index];
		if(val.template is<void*>()) {
			return val.get<void*>()
		}
	}*/


}
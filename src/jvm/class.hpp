#pragma once

#include "const_pool.hpp"
#include "trampoline_pool.hpp"
#include "object.hpp"
#include "../abort.hpp"
#include "class/file/access_flag.hpp"

#include <core/range.hpp>
#include <core/fixed_vector.hpp>
#include <stdio.h>

struct _class : const_pool, trampoline_pool {
private:
	span<uint8> data_;
	class_file::access_flags access_flags_;
	uint16 this_class_index_;
	uint16 super_class_index_;
	::fixed_vector<uint16, uint16, default_allocator> interfaces_;
	::fixed_vector<elements::one_of<field, static_field>, uint16, default_allocator> fields_;
	::fixed_vector<method, uint16, default_allocator> methods_;

	friend inline _class& define_class(span<uint8> bytes);

public:

	_class(const _class&) = delete;
	_class(_class&&) = default;

	inline _class(const_pool&& const_pool);
	inline ~_class();

	auto name() const {
		return utf8_constant(class_constant(this_class_index_).name_index);
	}

	template<range Name>
	inline method* try_find_method(Name name);

	template<range Name>
	inline field* try_find_field(Name name);

	template<range Name>
	method& find_method(Name&& name) {
		if(auto m = try_find_method(name); m != nullptr) {
			return *m;
		}
		fprintf(stderr, "couldn't find method %s", name.data());
		abort();
	}

	template<range Name>
	field& find_field(Name&& name) {
		if(auto f = try_find_field(name); f != nullptr) {
			return *f;
		}
		fprintf(stderr, "couldn't find field %s", name.data());
		abort();
	}

	inline method& get_method(uint16 ref_index);
	inline field& get_field(uint16 ref_index);

};

#include "field.hpp"
#include "method.hpp"

_class::_class(const_pool&& const_pool) :
	::const_pool{ move(const_pool) },
	::trampoline_pool{ constants_count() }
{}

_class::~_class() { free(data_.data()); }

template<range Name>
field* _class::try_find_field(Name name) {
	for(auto& f : fields_) if(equals(f.get<field>().name(), name))
		return &f.get<field>();
	return nullptr;
}

template<range Name>
method* _class::try_find_method(Name name) {
	for(auto& m : methods_) if(equals(m.name(), name)) return &m;
	return nullptr;
}

#include "class/get_field.hpp"
#include "class/get_method.hpp"
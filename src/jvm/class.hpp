#pragma once

#include "const_pool.hpp"
#include "trampoline_pool.hpp"
#include "field.hpp"
#include "static_field.hpp"
#include "../abort.hpp"
#include "class/file/access_flag.hpp"

#include <core/range.hpp>
#include <core/fixed_vector.hpp>
#include <core/transform.hpp>
#include <stdio.h>

struct _class : const_pool, trampoline_pool {
private:
	span<uint8> data_;
	class_file::access_flags access_flags_;
	uint16 this_class_index_;
	uint16 super_class_index_;
	::fixed_vector<uint16, uint16, default_allocator> interfaces_;
	::fixed_vector<
		elements::one_of<field, static_field
	>, uint16, default_allocator> fields_;
	::fixed_vector<
		field*, uint16, default_allocator
	> instance_fields_;
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

	uint16 super_class_index() const { return super_class_index_; }

	auto& interfaces_indices() const { return interfaces_; }

	auto interfaces() const {
		return transform_view {
			interfaces_indices(),
			[&](auto interface_index) -> decltype(auto) {
				return this->get_class(interface_index);
			}
		};
	}

	auto interfaces() {
		return transform_view {
			interfaces_indices(),
			[&](auto interface_index) -> decltype(auto) {
				return this->get_class(interface_index);
			}
		};
	}

	auto& methods() const { return methods_; }
	auto& methods() { return methods_; }

	class_file::access_flags access_flags() const {
		return access_flags_;
	}

	auto& instance_fields() const { return instance_fields_; }

	template<range Name>
	inline method* try_find_method(Name name);

	template<range Name, range Descriptor>
	inline const method*
	try_find_method(Name name, Descriptor descriptor) const;

	template<range Name, range Descriptor>
	inline method*
	try_find_method(Name name, Descriptor descriptor);

	template<range Name>
	inline field* try_find_field(Name name);

	template<range Name, range Descriptor>
	inline field* try_find_field(Name name, Descriptor descriptor);

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
	inline method& get_resolved_method(uint16 ref_index);
	inline field& get_static_field(uint16 ref_index);
	inline instance_field_index get_resolved_instance_field_index(uint16);
	inline _class& get_class(uint16 class_index);
	const _class& get_class(uint16 class_index) const {
		return ((_class*) this)->get_class(class_index);
	}

	template<typename Name, typename Descriptor, typename Handler>
	inline void for_each_maximally_specific_superinterface_method(
		Name&& name, Descriptor&& descriptor, Handler&& handler
	);

};

#include "method.hpp"

_class::_class(const_pool&& const_pool) :
	::const_pool{ move(const_pool) },
	::trampoline_pool{ constants_count() }
{}

_class::~_class() { free(data_.data()); }

template<range Name, range Descriptor>
field* _class::try_find_field(Name name, Descriptor descriptor) {
	for(auto& f0 : fields_) {
		auto& f = f0.get<field>();
		if(
			equals(f.name(), name) &&
			equals(f.descriptor(), descriptor)
		) return &f;
	}
	return nullptr;
}

template<range Name>
field* _class::try_find_field(Name name) {
	for(auto& f : fields_) if(equals(f.get<field>().name(), name))
		return &f.get<field>();
	return nullptr;
}

template<range Name, range Descriptor>
method* _class::try_find_method(Name name, Descriptor descriptor) {
	for(auto& m : methods_) {
		if(
			equals(m.name(), name) &&
			equals(m.descriptor(), descriptor)
		) return &m;
	}
	return nullptr;
}

template<range Name, range Descriptor>
const method* _class::try_find_method(Name name, Descriptor descriptor) const {
	return ((_class*)this)->try_find_method(
		forward<Name>(name), forward<Descriptor>(descriptor)
	);
}

template<range Name>
method* _class::try_find_method(Name name) {
	for(auto& m : methods_) if(equals(m.name(), name)) return &m;
	return nullptr;
}

#include "class/get_field.hpp"
#include "class/get_method.hpp"
#include "class/get_resolved_method.hpp"
#include "class/get_class.hpp"
#include "class/get_resolved_field.hpp"
#include "class/for_each_maximally_specific_superinterface_method.hpp"
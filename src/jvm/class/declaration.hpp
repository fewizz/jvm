#pragma once

#include "const_pool.hpp"
#include "trampoline_pool.hpp"
#include "../field/declaration.hpp"
#include "../field/static.hpp"
#include "../name_index.hpp"
#include "../../abort.hpp"
#include "class/file/access_flag.hpp"

#include <core/range.hpp>
#include <core/limited_list.hpp>
#include <core/transform.hpp>
#include <core/expected.hpp>
#include <core/c_string.hpp>
#include <core/meta/elements/optional.hpp>
#include <stdio.h>

template<typename... Args>
static inline _class& define_class0(Args&&... args);

template<range Name>
static inline _class& define_primitive_class(Name&& name);

struct _class : const_pool, trampoline_pool {
private:
	span<uint8> data_;
	class_file::access_flags access_flags_;
	name_index this_class_index_;
	name_index super_class_index_;
	::limited_list<uint16, uint16, default_allocator> interfaces_;
	::limited_list<
		elements::one_of<field, static_field
	>, uint16, default_allocator> fields_;
	::limited_list<
		field&, uint16, default_allocator
	> instance_fields_;
	::limited_list<method, uint16, default_allocator> methods_;
	reference class_class_;

	template<typename... Args>
	friend inline _class& define_class0(Args&&... args);

	friend inline _class& define_array_class(_class& element_class);

	template<range Name>
	friend inline _class& define_primitive_class(Name&& name);

public:

	_class(const _class&) = delete;
	_class(_class&&) = default;

	inline _class(const_pool&& const_pool);
	inline ~_class();

	auto name() const {
		return utf8_constant(class_constant(this_class_index_).name_index);
	}

	inline object& class_class();

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

	auto& fields() const { return fields_; }
	auto& fields() { return fields_; }

	auto& methods() const { return methods_; }
	auto& methods() { return methods_; }

	class_file::access_flags access_flags() const {
		return access_flags_;
	}

	auto& instance_fields() const { return instance_fields_; }
	auto& instance_fields() { return instance_fields_; }

	template<range Name>
	inline optional<method&> try_find_method(Name name);

	template<range Name, range Descriptor>
	inline const optional<method&>
	try_find_method(Name name, Descriptor descriptor) const;

	template<range Name, range Descriptor>
	inline optional<method&>
	try_find_method(Name name, Descriptor descriptor);

	template<range Name>
	inline optional<field&> try_find_field(Name name);

	template<range Name, range Descriptor>
	inline optional<field&> try_find_field(Name name, Descriptor descriptor);

	template<range Name, range Descriptor>
	inline optional<field_index>
	try_find_instance_field_index(Name name, Descriptor descriptor);

	template<range Name>
	method& find_method(Name&& name) {
		if(auto m = try_find_method(name); m.has_value()) {
			return m.value();
		}
		fprintf(stderr, "couldn't find method %s", name.data());
		abort();
	}

	template<range Name>
	field& find_field(Name&& name) {
		if(auto f = try_find_field(name); f.has_value()) {
			return f.value();
		}
		fprintf(stderr, "couldn't find field %s", name.data());
		abort();
	}

	inline method& get_method(uint16 ref_index);
	inline method& get_resolved_method(uint16 ref_index);
	inline field& get_static_field(uint16 ref_index);
	inline field_index get_resolved_instance_field_index(uint16);
	inline _class& get_class(uint16 class_index);
	const _class& get_class(uint16 class_index) const {
		return ((_class*) this)->get_class(class_index);
	}

	inline reference get_string(uint16 string_index);

	template<typename Name, typename Descriptor, typename Handler>
	inline void for_each_maximally_specific_superinterface_method(
		Name&& name, Descriptor&& descriptor, Handler&& handler
	);

};
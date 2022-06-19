#pragma once

#include "const_pool.hpp"
#include "trampoline_pool.hpp"
#include "../field/declaration.hpp"
#include "../field/static/declaration.hpp"
#include "../field/with_class/declaration.hpp"
#include "../name_index.hpp"
#include "../execute/declaration.hpp"
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

struct class_data : span<uint8> {
	using base_type = span<uint8>;
	using base_type::base_type;

	class_data(span<uint8> data) :
		base_type{ data }
	{}

	class_data(class_data&&) = delete;
	class_data(const class_data&) = delete;

	~class_data() {
		free(data());
	}
};

using interfaces_indices_container =
	::limited_list<
		uint16, uint16, default_allocator
	>;

using fields_container =
	::limited_list<
		elements::one_of<field, static_field>, uint16, default_allocator
	>;

using methods_container =
	::limited_list<
		method, uint16, default_allocator
	>;

struct this_class_index {
	uint16 _;
	operator uint16 () const { return _; }
};

struct super_class_index {
	uint16 _;
	operator uint16 () const { return _; }
};

struct _class : const_pool, trampoline_pool {
private:
	class_data                   data_{};
	class_file::access_flags     access_flags_{};
	this_class_index             this_class_index_{};
	super_class_index            super_class_index_{};
	interfaces_indices_container interfaces_{};
	fields_container             fields_{};
	methods_container            methods_{};
	reference                    reference_{};
	enum initialisation_state {
		not_initialised,
		pending,
		initialised
	} initialisation_state_ = not_initialised;

	template<typename... Args>
	friend inline _class& define_class0(Args&&... args);

	friend inline _class& define_array_class(_class& element_class);

	template<range Name>
	friend inline _class& define_primitive_class(Name&& name);

	friend struct classes_container;

	inline _class(
		const_pool&& const_pool,
		span<uint8> data, class_file::access_flags access_flags,
		this_class_index, super_class_index,
		interfaces_indices_container&& interfaces,
		fields_container&& fields, methods_container&& methods
	);

public:

	_class(_class&&) = delete;
	_class(const _class&) = delete;

	auto name() const {
		return utf8_constant(class_constant(this_class_index_).name_index);
	}

	inline object& object();

	bool has_super_class() const {
		return super_class_index_ != 0;
	}

	_class& super_class() {
		if(!has_super_class()) {
			fputs("no super class", stderr);
			abort();
		}
		return get_class(super_class_index_);
	}

	inline void initialise_if_need();

	auto name(const method& m) {
		return utf8_constant(m.name_index());
	}

	auto descriptor(const method& m) {
		return utf8_constant(m.descriptor_index());
	}

	auto name(const field& f) {
		return utf8_constant(f.name_index());
	}

	auto descriptor(const field& f) {
		return utf8_constant(f.descriptor_index());
	}

	const interfaces_indices_container& interfaces_indices() const {
		return interfaces_;
	}

	interfaces_indices_container& interfaces_indices() {
		return interfaces_;
	}

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

	inline method_with_class get_static_method(uint16 ref_index);
	inline method_with_class get_resolved_method(uint16 ref_index);
	inline static_field_with_class get_static_field(uint16 ref_index);
	inline field_index get_resolved_instance_field_index(uint16);
	inline _class& get_class(uint16 class_index);
	const _class& get_class(uint16 class_index) const {
		return ((_class*) this)->get_class(class_index);
	}

	template<typename Handler>
	inline void for_each_instance_field(Handler handler) {
		if(has_super_class()) {
			super_class().for_each_instance_field(handler);
		}
		for(auto& f : fields_) {
			if(f.is<field>()) {
				handler(field_with_class{ f.get<field>(), *this });
			}
		}
	}

	template<range Name, range Descriptor>
	optional<field_index>
	try_find_instance_field_index(
		Name name, Descriptor descriptor, uint16 index = 0
	);

	uint16 instance_fields_count() {
		uint16 count = 0;
		for_each_instance_field([&](auto) { ++count; });
		return count;
	}

	optional<field_with_class> instance_field(field_index index) {
		if(has_super_class()) {
			auto result = instance_field(index);
			if(result.has_value()) {
				return result;
			}
		}
		for(auto& f : fields_) {
			if(f.is<field>()) {
				if(index == 0) {
					return field_with_class{ f.get<field>(), *this };
				}
				--index._;
			}
		}
		return elements::none{};
	}

	inline reference get_string(uint16 string_index);

	template<typename Name, typename Descriptor, typename Handler>
	inline void for_each_maximally_specific_superinterface_method(
		Name&& name, Descriptor&& descriptor, Handler&& handler
	);

};
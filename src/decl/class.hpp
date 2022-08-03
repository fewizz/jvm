#pragma once

#include "class/interfaces_indices_container.hpp"
#include "class/instance_fields_container.hpp"
#include "class/static_fields_container.hpp"
#include "class/methods_container.hpp"
#include "class/this_class_index.hpp"
#include "class/super_class_index.hpp"
#include "class/const_pool.hpp"
#include "class/trampoline_pool.hpp"
#include "class/bootstrap_method_pool.hpp"
#include "class/is_array.hpp"
#include "class/is_primitive.hpp"

#include "field.hpp"
#include "field/instance.hpp"
#include "field/instance/with_class.hpp"
#include "field/static.hpp"
#include "field/static/with_class.hpp"
#include "execute.hpp"
#include "abort.hpp"

#include <class_file/access_flag.hpp>

#include <core/range.hpp>
#include <core/limited_list.hpp>
#include <core/transform.hpp>
#include <core/expected.hpp>
#include <core/c_string.hpp>
#include <core/optional.hpp>

#include <stdio.h>

struct class_bytes : span<uint8> {
	using base_type = span<uint8>;
	using base_type::base_type;

	class_bytes(span<uint8> data) :
		base_type{ data }
	{}

	class_bytes(class_bytes&&) = delete;
	class_bytes(const class_bytes&) = delete;

	~class_bytes() {
		free(data());
	}
};

struct _class : const_pool, trampoline_pool, bootstrap_method_pool {
private:
	class_bytes                  bytes_{};
	class_file::access_flags     access_flags_;
	this_class_index             this_class_index_;
	super_class_index            super_class_index_;
	interfaces_indices_container interfaces_;
	instance_fields_container    instance_fields_;
	static_fields_container      static_fields_;
	methods_container            methods_;
	optional<_class&>            array_class_;
	optional<_class&>            component_class_;
	is_array_class               is_array_class_;
	is_primitive_class           is_primitive_class_;
	reference                    instance_{};
	enum initialisation_state {
		not_initialised,
		pending,
		initialised
	} initialisation_state_ = not_initialised;

public:

	_class(
		const_pool&&, bootstrap_method_pool&&,
		span<uint8> data, class_file::access_flags,
		this_class_index, super_class_index,
		interfaces_indices_container&&,
		instance_fields_container&&,
		static_fields_container&&,
		methods_container&&,
		is_array_class,
		is_primitive_class
	);

	_class(_class&&) = delete;
	_class(const _class&) = delete;
	_class& operator = (_class&&) = delete;
	_class& operator = (const _class&) = delete;

	class_file::constant::utf8 name() const {
		return utf8_constant(class_constant(this_class_index_).name_index);
	}

	class_file::access_flags access_flags() const {
		return access_flags_;
	}

	reference instance();

	bool has_super_class() const {
		return super_class_index_ != 0;
	}

	bool is_interface() const {
		return access_flags().get(class_file::access_flag::interface);
	}

	_class& super_class() {
		if(!has_super_class()) {
			fputs("no super class", stderr);
			abort();
		}
		return get_class(super_class_index_);
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

	auto& declared_instance_fields() const { return instance_fields_; }
	auto& declared_instance_fields() { return instance_fields_; }

	auto& declared_static_fields() const { return static_fields_; }
	auto& declared_static_fields() { return static_fields_; }

	auto& methods() const { return methods_; }
	auto& methods() { return methods_; }

	bool is_initialised() const {
		return initialisation_state_ == initialisation_state::initialised;
	}

	void initialise_if_need();

	bool is_subclass_of(_class& other) {
		if(has_super_class()) {
			_class& super = super_class();
			if(&super == &other) {
				return true;
			}
			return super.is_subclass_of(other);
		}
		return false;
	}

	bool is_implements(_class& other) {
		for(_class& i : interfaces()) {
			if(&i == &other) {
				return true;
			}
			if(i.is_implements(other)) {
				return true;
			}
		}
		return false;
	}

	class_file::constant::utf8 name(const class_member& m) {
		return utf8_constant(m.name_index());
	}

	class_file::constant::utf8 descriptor(const class_member& f) {
		return utf8_constant(f.descriptor_index());
	}

	uint16 instance_fields_count() {
		uint16 count = declared_instance_fields().size();
		if(has_super_class()) {
			count += super_class().instance_fields_count();
		}
		return count;
	}

	template<typename Handler>
	void for_each_instance_field(Handler&& handler) {
		if(has_super_class()) {
			_class& super = super_class();
			super.for_each_instance_field(handler);
		}
		for(instance_field& field : declared_instance_fields()) {
			handler(instance_field_with_class{ field, *this });
		}
	}

	template<range Name>
	optional<method&> try_find_method(Name&& name);

	template<range Name, range Descriptor>
	optional<method&>
	try_find_method(Name&& name, Descriptor&& descriptor);

	template<range Name>
	method& find_method(Name&& name) {
		if(auto m = try_find_method(name); m.has_value()) {
			return m.value();
		}
		fputs("couldn't find method ", stderr);
		fwrite(name.data(), 1, name.size(), stderr);
		abort();
	}

	template<range Name, range Descriptor>
	method& find_method(Name&& name, Descriptor&& desc) {
		if(auto m = try_find_method(name, desc); m.has_value()) {
			return m.value();
		}
		fputs("couldn't find method ", stderr);
		fwrite(name.data(), 1, name.size(), stderr);
		abort();
	}

	template<range Name>
	optional<declared_instance_field_index>
	try_find_declared_instance_field_index(Name&& name);

	template<range Name>
	optional<instance_field&>
	try_find_declared_instance_field(Name&& name);

	template<range Name>
	declared_instance_field_index
	find_declared_instance_field_index(Name&& name);

	template<range Name>
	instance_field&
	find_declared_instance_field(Name&& name);

	template<range Name, range Descriptor>
	optional<instance_field_index>
	try_find_instance_field_index(Name&& name, Descriptor&& descriptor);

	template<range Name, range Descriptor>
	instance_field_index
	find_instance_field_index(Name&& name, Descriptor&& descriptor) {
		optional<instance_field_index> result {
			try_find_instance_field_index(name, descriptor)
		};
		if(!result.has_value()) {
			fputs("couldn't find instance field ", stderr);
			auto class_name = this->name();
			fwrite(class_name.data(), 1, class_name.size(), stderr);
			fputc('.', stderr);
			fwrite(name.data(), 1, name.size(), stderr);
			abort();
		}
		return result.value();
	}

	optional<instance_field_with_class>
	try_get_instance_field(instance_field_index index);

	template<range Name, range Descriptor>
	optional<static_field&>
	try_find_declared_static_field(Name&& name, Descriptor&& descriptor);

	template<range Name, range Descriptor>
	static_field&
	find_declared_static_field(Name&& name, Descriptor&& descriptor);

	method_with_class get_static_method(
		class_file::constant::method_ref_index ref_index
	);
	method_with_class get_resolved_method(
		class_file::constant::method_ref_index ref_index
	);
	static_field_with_class get_static_field(
		class_file::constant::field_ref_index ref_index
	);
	instance_field_index get_resolved_instance_field_index(
		class_file::constant::field_ref_index
	);
	_class& get_class(class_file::constant::class_index class_index);

	::reference get_string(class_file::constant::string_index string_index);

	template<typename Name, typename Descriptor, typename Handler>
	void for_each_maximally_specific_superinterface_method(
		Name&& name, Descriptor&& descriptor, Handler&& handler
	);

	_class& get_array_class();

	_class& get_component_class();

	bool is_array_class() const { return is_array_class_; }
	bool is_primitive_class() const { return is_primitive_class_; }

	void unsafe_set_array_class(_class& c) {
		array_class_ = c;
	}

	void unsafe_set_component_class(_class& c) {
		component_class_ = c;
	}

};
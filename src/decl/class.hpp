#pragma once

#include "class/fields_container.hpp"
#include "class/methods_container.hpp"
#include "class/interfaces_indices_container.hpp"
#include "class/instance_fields_container.hpp"
#include "class/instance_field_index.hpp"
#include "class/static_fields_container.hpp"
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

#include <range.hpp>
#include <memory_list.hpp>
#include <expected.hpp>
#include <c_string.hpp>
#include <optional.hpp>
#include <loop_action.hpp>

#include <stdio.h>

struct _class : const_pool, trampoline_pool, bootstrap_method_pool {
private:
	memory_span                  bytes_{};
	class_file::access_flags     access_flags_;
	this_class_index             this_class_index_;
	super_class_index            super_class_index_;
	interfaces_indices_container interfaces_;

	fields_container             declared_fields_;
	memory_list<field&, uint16>  declared_instance_fields_;
	memory_list<field&, uint16>  declared_static_fields_;
	memory_list<field&, uint16>  instance_fields_;

	methods_container            declared_methods_;
	memory_list<method&, uint16> declared_instance_methods_;
	memory_list<method&, uint16> declared_static_methods_;
	memory_list<method&, uint16> instance_methods_;

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
		const_pool&&,
		trampoline_pool&& trampoline_pool,
		bootstrap_method_pool&&,
		memory_span bytes, class_file::access_flags,
		this_class_index, super_class_index,
		interfaces_indices_container&&,
		fields_container&&,
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

	auto interfaces() {
		return ::range{ interfaces_indices() }.transform_view(
			[&](auto interface_index) -> decltype(auto) {
				return this->get_class(interface_index);
			}
		);
	}

	template<typename Handler>
	void for_each_superinterface(Handler&& handler) {
		for(_class& i : interfaces()) {
			loop_action action = handler(i);
			switch (action) {
				case loop_action::stop: return;
				case loop_action::next: i.for_each_superinterface(handler);
			}
		}
	}

	auto& declared_fields() const & { return declared_fields_; }
	auto& declared_fields()       & { return declared_fields_; }

	auto& declared_instance_fields() const & { return instance_fields_; }
	auto& declared_instance_fields()       & { return instance_fields_; }

	auto& declared_static_fields() const & { return declared_static_fields_; }
	auto& declared_static_fields()       & { return declared_static_fields_; }

	auto& instance_fields() const & { return instance_fields_; }
	auto& instance_fields()       & { return instance_fields_; }

	auto& declared_methods() const & { return declared_methods_; }
	auto& declared_methods()       & { return declared_methods_; }

	auto& declared_instance_methods() const & {
		return declared_instance_methods_;
	}
	auto& declared_instance_methods()       & {
		return declared_instance_methods_;
	}

	auto& declared_static_methods() const & { return declared_static_methods_; }
	auto& declared_static_methods()       & { return declared_static_methods_; }

	auto& instance_methods() const & { return instance_methods_; }
	auto& instance_methods()       & { return instance_methods_; }

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

	bool is_implementing(_class& other) {
		for(_class& i : interfaces()) {
			if(&i == &other) {
				return true;
			}
			if(i.is_implementing(other)) {
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
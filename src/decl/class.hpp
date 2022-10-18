#pragma once

#include "./class/declared_fields.hpp"
#include "./class/declared_instance_fields.hpp"
#include "./class/declared_static_fields.hpp"
#include "./class/instance_fields.hpp"

#include "./class/declared_methods.hpp"
#include "./class/declared_instance_methods.hpp"
#include "./class/declared_static_methods.hpp"
#include "./class/instance_methods.hpp"

#include "./class/constants.hpp"
#include "./class/trampolines.hpp"
#include "./class/bootstrap_methods.hpp"

#include "./class/declared_interfaces.hpp"

#include "./class/name.hpp"
#include "./class/is_array.hpp"
#include "./class/is_primitive.hpp"

#include "./field/value.hpp"

#include "./object/reference.hpp"

#include <class_file/access_flag.hpp>

struct _class : constants, trampolines, bootstrap_methods {
private:
	posix::memory_for_range_of<uint8> bytes_;
	class_file::access_flags     access_flags_;
	this_class_name              this_name_;
	posix::memory_for_range_of<uint8> descriptor_;
	optional<_class&>            super_;

	::declared_interfaces        declared_interfaces_;

	::declared_fields            declared_fields_;
	::declared_instance_fields   declared_instance_fields_;
	::declared_static_fields     declared_static_fields_;
	::instance_fields            instance_fields_;

	list<posix::memory_for_range_of<
		field_value
	>>                           declared_static_fields_values_;

	::declared_methods           declared_methods_;
	::declared_instance_methods  declared_instance_methods_;
	::declared_static_methods    declared_static_methods_;
	::instance_methods           instance_methods_;

	optional<_class&>            array_class_;
	optional<_class&>            component_class_;
	reference                    instance_;

	is_array_class               is_array_;
	is_primitive_class           is_primitive_;
	enum initialisation_state {
		not_started,
		pending,
		done
	}                            initialisation_state_ = not_started;

public:

	_class(
		constants&&, bootstrap_methods&&,
		posix::memory_for_range_of<uint8> bytes, class_file::access_flags,
		this_class_name,
		posix::memory_for_range_of<uint8> descriptor,
		optional<_class&> super,
		::declared_interfaces&&,
		::declared_fields&&,
		::declared_methods&&,
		is_array_class,
		is_primitive_class
	);

	_class(_class&&) = delete;
	_class(const _class&) = delete;
	_class& operator = (_class&&) = delete;
	_class& operator = (const _class&) = delete;

	class_file::access_flags access_flags() const { return access_flags_; }

	this_class_name name() const { return this_name_; }
	span<char> descriptor() const {
		return span{ (char*) descriptor_.iterator(), descriptor_.size() };
	}
	_class& super() { return super_.value(); }
	bool has_super() const { return super_.has_value(); }

	bool is_interface() const { return access_flags_.interface; }

	bool is_array() const { return is_array_; }
	bool is_primitive() const { return is_primitive_; }
	bool is_reference() const { return !is_primitive_; }
	bool is(const _class& c) const { return &c == this; }

	_class& get_array_class();
	_class& get_component_class();

	void initialise_if_need();

	auto& declared_fields() { return declared_fields_; }
	auto& declared_static_fields() { return declared_static_fields_; }
	auto& instance_fields() { return instance_fields_; }

	auto& declared_static_fields_values() {
		return declared_static_fields_values_;
	}

	auto& declared_methods() { return declared_methods_; }
	auto& declared_static_methods() { return declared_static_methods_; }
	auto& declared_instance_methods() { return declared_instance_methods_; }
	auto& instance_methods() { return instance_methods_; }

	auto& declared_interfaces() { return declared_interfaces_; }

	reference instance();

	void array_class(_class& c)     { array_class_     = c; }
	void component_class(_class& c) { component_class_ = c; }

	reference get_string(
		class_file::constant::string_index string_index
	);

	_class& get_resolved_class(
		class_file::constant::class_index string_index
	);

	reference get_resolved_method_handle(
		class_file::constant::method_handle_index index
	);

	reference get_resolved_call_site(
		class_file::constant::invoke_dynamic_index index
	);

	method& get_resolved_method(
		class_file::constant::method_ref_index ref_index
	);

	method& resolve_method(
		class_file::constant::method_ref
	);

	method& resolve_interface_method(
		class_file::constant::interface_method_ref ref
	);

	tuple<instance_field_index, _class&>
	get_resolved_instance_field_index_and_class(
		class_file::constant::field_ref_index ref_index
	);

	method& get_static_method(
		class_file::constant::method_ref_index ref_index
	);

	field_value& get_static_field_value(
		class_file::constant::field_ref_index ref_index
	);

	template<typename Name, typename Descriptor, typename Handler>
	void for_each_maximally_specific_super_interface_instance_method(
		Name&& name, Descriptor&& descriptor, Handler&& handler
	);

	template<typename Handler>
	void for_each_super_interface(Handler&& handler);
	
	bool is_sub_of(_class& other) {
		if(has_super()) {
			_class& s = super();
			if(&s == &other) {
				return true;
			}
			return s.is_sub_of(other);
		}
		return false;
	}

	bool is_implementing(_class& other) {
		for(_class* i : declared_interfaces()) {
			if(i == &other) {
				return true;
			}
			if(i->is_implementing(other)) {
				return true;
			}
		}
		return false;
	}

};
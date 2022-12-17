#pragma once

#include "./class/layout.hpp"

#include "./class/constants.hpp"
#include "./class/trampolines.hpp"
#include "./class/bootstrap_methods.hpp"

#include "./class/name.hpp"
#include "./class/is_array.hpp"
#include "./class/is_primitive.hpp"

#include "./class/find_by_name_and_descriptor_extension.hpp"

#include "./field/value.hpp"

#include "./reference.hpp"

#include <class_file/access_flag.hpp>

struct _class : constants, trampolines, bootstrap_methods {
private:
	const posix::memory_for_range_of<uint8> bytes_;
	const class_file::access_flags access_flags_;
	const this_class_name this_name_;
	const posix::memory_for_range_of<uint8> descriptor_;
	optional<_class&> super_;

	const posix::memory_for_range_of<_class*> declared_interfaces_;
	posix::memory_for_range_of<field> declared_fields_;
	posix::memory_for_range_of<method> declared_methods_;

	const posix::memory_for_range_of<field*> declared_static_fields_;
	const posix::memory_for_range_of<method*> declared_static_methods_;

	const posix::memory_for_range_of<field*> declared_instance_fields_;
	const posix::memory_for_range_of<method*> declared_instance_methods_;

	const posix::memory_for_range_of<field*> instance_fields_;
	const posix::memory_for_range_of<method*> instance_methods_;

	const layout instance_layout_;
	const layout static_layout_;

	optional<_class&> array_class_;
	optional<_class&> component_class_;
	reference instance_;

	const is_array_class is_array_;
	const is_primitive_class is_primitive_;
	posix::memory_for_range_of<
		field_value
	> declared_static_fields_values_;
	enum initialisation_state {
		not_started,
		pending,
		done
	} initialisation_state_ = not_started;

public:

	_class(
		constants&&, bootstrap_methods&&,
		posix::memory_for_range_of<uint8> bytes, class_file::access_flags,
		this_class_name,
		posix::memory_for_range_of<uint8> descriptor,
		optional<_class&> super,
		posix::memory_for_range_of<_class*>,
		posix::memory_for_range_of<field>,
		posix::memory_for_range_of<method>,
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
		return ::span{ (char*) descriptor_.iterator(), descriptor_.size() };
	}
	const _class& super() const { return super_.get(); }
	      _class& super()       { return super_.get(); }
	bool has_super() const { return super_.has_value(); }

	bool is_interface() const { return access_flags_.interface; }

	bool is_array() const { return is_array_; }
	bool is_primitive() const { return is_primitive_; }
	bool is_not_primitive() const { return !is_primitive_; }
	bool is_reference() const { return !is_primitive_; }
	bool is(const _class& c) const { return &c == this; }

	_class& get_array_class();
	_class& get_component_class();

	void initialise_if_need();

	const layout& instance_layout() const { return instance_layout_; }
	const layout& static_layout() const { return static_layout_; }

	layout::position instance_field_position(
		instance_field_index index
	) {
		return instance_layout().slot_for_field_index(index).beginning();
	}

	template<basic_range Name, basic_range Descriptor>
	layout::position instance_field_position(
		Name&& name, Descriptor&& descriptor
	);

	auto declared_fields() {
		return declared_fields_.as_span();
	}
	auto declared_methods() {
		return find_by_name_and_descriptor_view {
			declared_methods_.as_span()
		};
	}

	auto declared_static_fields() const {
		return find_by_name_and_descriptor_view {
			declared_static_fields_.as_span().dereference_view()
		};
	}
	auto instance_fields() const {
		return find_by_name_and_descriptor_view {
			instance_fields_.as_span().dereference_view()
		};
	}

	auto declared_static_fields_values() {
		return declared_static_fields_values_.as_span();
	}

	auto declared_static_methods() const {
		return find_by_name_and_descriptor_view {
			declared_static_methods_.as_span().dereference_view()
		};
	}

	auto declared_instance_methods() const {
		return find_by_name_and_descriptor_view {
			declared_instance_methods_.as_span().dereference_view()
		};
	}
	auto declared_instance_fields() const {
		return find_by_name_and_descriptor_view {
			declared_instance_fields_.as_span().dereference_view()
		};
	}
	auto instance_methods() const {
		return find_by_name_and_descriptor_view {
			instance_methods_.as_span().dereference_view()
		};
	}

	auto declared_interfaces() const {
		return declared_interfaces_.transform_view(
			[](storage<_class*>& storage) -> _class& { return *storage.get(); }
		);
	}

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

	bool is_sub_of(_class& other) const {
		if(has_super()) {
			const _class& s = super();
			if(&s == &other) {
				return true;
			}
			return s.is_sub_of(other);
		}
		return false;
	}

	bool is_implementing(_class& other) const {
		for(_class& i : declared_interfaces()) {
			if(&i == &other) {
				return true;
			}
			if(i.is_implementing(other)) {
				return true;
			}
		}
		return false;
	}

};
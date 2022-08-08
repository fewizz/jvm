#pragma once

#include "./class/constants.hpp"
#include "./class/trampolines.hpp"
#include "./class/bootstrap_methods.hpp"

#include "./class/declared_fields.hpp"
#include "./class/declared_methods.hpp"

#include "./class/is_array.hpp"
#include "./class/is_primitive.hpp"

#include "object/reference.hpp"

#include <class_file/access_flag.hpp>
#include <memory_list.hpp>

struct _class : constants, trampolines, bootstrap_methods {
private:
	memory_span                  bytes_{};
	class_file::access_flags     access_flags_;
	struct this_name : public class_file::constant::utf8 {
		using class_file::constant::utf8::utf8;
	}                            this_name_;
	struct super_name : public class_file::constant::utf8 {
		using class_file::constant::utf8::utf8;
	}                            super_name_;

	memory_list<_class&, uint16> interfaces_;

	::declared_fields            declared_fields_;
	memory_list<field&, uint16>  declared_instance_fields_;
	memory_list<field&, uint16>  declared_static_fields_;
	memory_list<field&, uint16>  instance_fields_;

	::declared_methods           declared_methods_;
	memory_list<method&, uint16> declared_instance_methods_;
	memory_list<method&, uint16> declared_static_methods_;
	memory_list<method&, uint16> instance_methods_;

	optional<_class&>            array_class_;
	optional<_class&>            component_class_;
	reference                    instance_{};

	is_array_class               is_array_;
	is_primitive_class           is_primitive_;
	enum initialisation_state {
		not_initialised,
		pending,
		initialised
	}                            initialisation_state_ = not_initialised;

public:

	_class(
		constants&&, trampolines&&, bootstrap_methods&&,
		memory_span bytes, class_file::access_flags,
		this_name, super_name,
		memory_list<_class&, uint16>&& interfaces,
		declared_fields&&,
		declared_methods&&,
		is_array_class,
		is_primitive_class
	);

	_class(_class&&) = delete;
	_class(const _class&) = delete;
	_class& operator = (_class&&) = delete;
	_class& operator = (const _class&) = delete;

	auto name() const { return this_name_; }

	bool is_array() { return is_array_; }

	_class& get_array_class();
	_class& get_component_class();

};
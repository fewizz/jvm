#pragma once

#include "./method/code.hpp"
#include "./parameters_count.hpp"
#include "./class/member.hpp"
#include "./alloc.hpp"

#include <class_file/access_flag.hpp>
#include <class_file/constant.hpp>
#include <class_file/descriptor/type.hpp>
#include <class_file/descriptor/reader.hpp>
#include <class_file/attribute/code/exception_handler.hpp>

#include <span.hpp>
#include <elements/one_of.hpp>
#include <optional.hpp>
#include <memory_list.hpp>
#include <c_string.hpp>

struct native_function_ptr {
private:
	void* value_;
public:
	native_function_ptr(void* value) : value_{ value } {}

	operator void*& () & { return value_; }
};

using code_or_native_function_ptr =
	elements::one_of<code, optional<native_function_ptr>>;

using exception_handlers = memory_list<
	class_file::attribute::code::exception_handler, uint16
>;

using one_of_descriptor_types = elements::one_of<
	class_file::descriptor::Z,
	class_file::descriptor::B,
	class_file::descriptor::C,
	class_file::descriptor::S,
	class_file::descriptor::I,
	class_file::descriptor::F,
	class_file::descriptor::J,
	class_file::descriptor::D,
	class_file::descriptor::array_type,
	class_file::descriptor::object_type,
	class_file::descriptor::V
>;

struct _class;

struct parameter_type : one_of_descriptor_types {
private:
	using base_type = one_of_descriptor_types;
public:
	using base_type::base_type;
};

struct return_type : one_of_descriptor_types {
private:
	using base_type = one_of_descriptor_types;
public:
	using base_type::base_type;

	return_type() : base_type{ class_file::descriptor::V{} } {}
};

struct method : class_member {
private:
	using base_type = class_member;

	memory_list<parameter_type, uint8> parameters_types_;
	return_type                        return_type_;
	code_or_native_function_ptr        code_;
	exception_handlers                 exception_handlers_;

public:

	method(
		class_file::access_flags               access_flags,
		class_file::constant::utf8             name,
		class_file::constant::utf8             descriptor,
		code_or_native_function_ptr            code,
		exception_handlers&&                   exception_handlers
	) :
		base_type          { access_flags, name, descriptor },
		code_              { code                           },
		exception_handlers_{ move(exception_handlers)       }
	{
		class_file::descriptor::method_reader mr {
			descriptor.iterator()
		};

		uint8 count = 0;
		mr([&](auto) {
			++count;
			return true;
		});
		parameters_types_ = { allocate_for<parameter_type>(count) }; 

		auto [return_type_reader, res] = mr([&](auto parameter_type) {
			parameters_types_.emplace_back(parameter_type);
			return true;
		});

		return_type_reader([&](auto ret_type) {
			return_type_ = ret_type;
			return true;
		});
	}

	parameters_count parameters_count() {
		return ::parameters_count{ parameters_types_.size() };
	}

	code code() const { return code_.get<::code>(); }

	exception_handlers& exception_handlers() {
		return exception_handlers_;
	}

	bool is_native() const {
		return access_flags().native();
	}

	bool native_function_is_loaded() const {
		return code_.get<optional<native_function_ptr>>().has_value();
	}

	void native_function(native_function_ptr function) {
		code_ = function;
	}

	native_function_ptr native_function() {
		return code_.get<optional<native_function_ptr>>().value();
	}

	bool is_void() const {
		return return_type_.is<class_file::descriptor::V>();
	}

	bool is_instance_initialisation() const;

};
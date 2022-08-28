#pragma once

#include "./method/code.hpp"
#include "./parameters_count.hpp"
#include "./class/member.hpp"
#include "./alloc.hpp"
#include "./method_descriptor.hpp"

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

struct method : class_member<method_descriptor<class_file::constant::utf8>> {
private:
	using base_type = class_member<
		method_descriptor<class_file::constant::utf8>
	>;

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
		base_type          {
			access_flags, name, method_descriptor{ move(descriptor) }
		},
		code_              { code                           },
		exception_handlers_{ move(exception_handlers)       }
	{}

	parameters_count parameters_count() {
		return ::parameters_count{ descriptor().parameters_types().size() };
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
		return descriptor().return_type().is<class_file::descriptor::V>();
	}

	bool is_instance_initialisation() const;

};
#pragma once

#include "./method/code.hpp"
#include "./parameters_count.hpp"
#include "./class/member.hpp"
#include "./method_descriptor.hpp"

#include <class_file/access_flag.hpp>
#include <class_file/constant.hpp>
#include <class_file/descriptor/type.hpp>
#include <class_file/descriptor/method_reader.hpp>
#include <class_file/attribute/code/exception_handler.hpp>

#include <span.hpp>
#include <variant.hpp>
#include <optional.hpp>
#include <list.hpp>
#include <c_string.hpp>

#include <posix/memory.hpp>

struct native_function_ptr {
private:
	void* value_;
public:
	native_function_ptr(void* value) : value_{ value } {}

	operator void*& () & { return value_; }
};

using code_or_native_function_ptr =
	variant<code, optional<native_function_ptr>>;

using exception_handlers = list<
	posix::memory_for_range_of<class_file::attribute::code::exception_handler>
>;

struct method : class_member<method_descriptor<class_file::constant::utf8>> {
private:
	using base_type = class_member<
		method_descriptor<class_file::constant::utf8>
	>;

	code_or_native_function_ptr        code_;
	exception_handlers                 exception_handlers_;
	uint8                              parameters_stack_size_ = 0;

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
	{
		if(!access_flags._static) {
			++parameters_stack_size_; // this
		}
		for(auto t : base_type::descriptor().parameters_types()) {
			t.view([&]<typename Type>(Type) {
				if constexpr(
					same_as<Type, class_file::j> ||
					same_as<Type, class_file::d>
				) {
					parameters_stack_size_ += 2;
				}
				else {
					parameters_stack_size_ += 1;
				}
			});
		}
	}

	uint8 parameters_stack_size() const { return parameters_stack_size_; }

	parameters_count parameters_count() {
		auto count = descriptor().parameters_types().size();
		return ::parameters_count{ (uint8) count };
	}

	code code() const { return code_.get<::code>(); }

	exception_handlers& exception_handlers() {
		return exception_handlers_;
	}

	bool is_native() const {
		return access_flags().native;
	}

	bool native_function_is_loaded() const {
		return code_.get<optional<native_function_ptr>>().has_value();
	}

	void native_function(native_function_ptr function) {
		if(!code_.is<optional<native_function_ptr>>()) {
			abort();
		}
		code_ = function;
	}

	native_function_ptr native_function() {
		return code_.get<optional<native_function_ptr>>().value();
	}

	bool is_void() const {
		return descriptor().return_type().is<class_file::v>();
	}

	bool is_instance_initialisation() const;

};
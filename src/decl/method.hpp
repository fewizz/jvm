#pragma once

#include "decl/method/code.hpp"
#include "decl/parameters_count.hpp"
#include "decl/class/member.hpp"

#include <class_file/access_flag.hpp>
#include <class_file/constant.hpp>
#include <class_file/attribute/code/exception_handler.hpp>

#include <span.hpp>
#include <elements/one_of.hpp>
#include <optional.hpp>
#include <memory_list.hpp>
#include <c_string.hpp>

struct _class;
struct native_function;

using code_or_native_function =
	elements::one_of<code, optional<void*>>;

using exception_handlers = memory_list<
	class_file::attribute::code::exception_handler, uint16
>;

using parameters_type_names = memory_list<
	c_string_of_known_size,
	uint8
>;

struct method : class_member {
private:
	using base_type = class_member;

	parameters_type_names   parameters_names_;
	code_or_native_function code_;
	exception_handlers      exception_handlers_;

public:

	method(
		class_file::access_flags               access_flags,
		class_file::constant::utf8             name,
		class_file::constant::utf8             descriptor,
		parameters_type_names                  parameters_names,
		code_or_native_function                code,
		exception_handlers&&         exception_handlers
	) :
		base_type          { access_flags, name, descriptor },
		parameters_names_  { move(parameters_names)         },
		code_              { code                           },
		exception_handlers_{ move(exception_handlers)       }
	{}

	parameters_count parameters_count() {
		return ::parameters_count{ parameters_names_.size() };
	}

	code code() const { return code_.get<::code>(); }

	const parameters_type_names& parameters_names() {
		return parameters_names_;
	}

	exception_handlers& exception_handlers() {
		return exception_handlers_;
	}

	bool is_native() const {
		return access_flags().native();
	}

	bool native_function_is_loaded() const {
		return code_.get<optional<void*>>().has_value();
	}

	void native_function(void* function) {
		code_ = function;
	}

	void* native_function() {
		return code_.get<optional<void*>>().value();
	}

};
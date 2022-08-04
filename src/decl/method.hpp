#pragma once

#include "method/code.hpp"
#include "parameters_count.hpp"
#include "class/member.hpp"
#include "native/function.hpp"

#include <class_file/access_flag.hpp>
#include <class_file/constant.hpp>
#include <class_file/attribute/code/exception_handler.hpp>

#include <core/span.hpp>
#include <core/meta/elements/one_of.hpp>

struct _class;

using code_or_native_function =
	elements::one_of<code, optional<native_function&>>;

using exception_handlers_container = limited_list<
	class_file::attribute::code::exception_handler,
	uint16,
	default_allocator
>;

using parameter_type_names_container = limited_list<
	c_string<c_string_type::known_size>,
	uint8,
	default_allocator
>;

struct method : class_member {
private:
	using base_type = class_member;

	parameter_type_names_container parameter_names_;
	code_or_native_function        code_;
	exception_handlers_container   exception_handlers_;

public:

	method(
		class_file::access_flags               access_flags,
		class_file::constant::name_index       name_index,
		class_file::constant::descriptor_index descriptor_index,
		parameter_type_names_container         paramerter_names,
		code_or_native_function                code,
		exception_handlers_container&&         exception_handlers
	) :
		base_type          { access_flags, name_index, descriptor_index },
		parameter_names_   { move(paramerter_names)                     },
		code_              { code                                       },
		exception_handlers_{ move(exception_handlers)                   }
	{}

	parameters_count parameters_count() {
		return ::parameters_count{ parameter_names_.size() };
	}

	code code() const { return code_.get<::code>(); }

	const parameter_type_names_container& parameter_names() {
		return parameter_names_;
	}

	exception_handlers_container& exception_handlers() {
		return exception_handlers_;
	}

	bool is_native() const {
		return access_flags().native();
	}

	bool native_function_is_loaded() const {
		return code_.get<optional<::native_function&>>().has_value();
	}

	void native_function(::native_function& function) {
		code_ = function;
	}

	::native_function& native_function() {
		return code_.get<optional<::native_function&>>().value();
	}

};
#pragma once

#include "method/code.hpp"
#include "arguments_count.hpp"
#include "class/member.hpp"
#include "native/function.hpp"

#include <class_file/access_flag.hpp>
#include <class_file/constant.hpp>
#include <class_file/attribute/code/exception_handler.hpp>

#include <core/span.hpp>
#include <core/meta/elements/one_of.hpp>

struct _class;

using code_or_native_function =
	elements::one_of<elements::none, code, native_function&>;

using exception_handlers_container = limited_list<
	class_file::attribute::code::exception_handler,
	uint16,
	default_allocator
>;

struct method : class_member {
private:
	using base_type = class_member;

	arguments_count              arguments_count_;
	code_or_native_function      code_;
	exception_handlers_container exception_handlers_;

public:

	method(
		class_file::access_flags               access_flags,
		class_file::constant::name_index       name_index,
		class_file::constant::descriptor_index descriptor_index,
		arguments_count                        arguments_count,
		code_or_native_function                code,
		exception_handlers_container&&         exception_handlers
	) :
		base_type          { access_flags, name_index, descriptor_index },
		arguments_count_   { arguments_count                            },
		code_              { code                                       },
		exception_handlers_{ move(exception_handlers)                   }
	{}

	arguments_count arguments_count() { return arguments_count_; }

	code code() const { return code_.get<::code>(); }

	exception_handlers_container& exception_handlers() {
		return exception_handlers_;
	}

	bool is_native() const {
		return access_flags().native();
	}

	bool has_native_function() const {
		return code_.is<::native_function&>();
	}

	void native_function(::native_function& function) {
		code_ = function;
	}

	::native_function& native_function() {
		return code_.get<::native_function&>();
	}

};
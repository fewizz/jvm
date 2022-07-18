#pragma once

#include "../class/member/decl.hpp"
#include "../native/function/decl.hpp"
#include "../descriptor_index.hpp"
#include "../name_index.hpp"

#include <class/file/access_flag.hpp>
#include <class/file/constant.hpp>
#include <class/file/attribute/code/exception_handler.hpp>
#include <core/span.hpp>
#include <core/meta/elements/one_of.hpp>

struct _class;

struct code : span<uint8, uint32> {
	using base_type = span<uint8, uint32>;

	uint16 max_stack;
	uint16 max_locals;

	code() = default;

	code(span<uint8, uint32> bytes, uint16 max_stack, uint16 max_locals) :
		base_type{ bytes },
		max_stack{ max_stack },
		max_locals{ max_locals }
	{}

};

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

	code_or_native_function  code_;
	exception_handlers_container exception_handlers_;

public:

	method(
		class_file::access_flags       access_flags,
		::name_index                   name_index,
		::descriptor_index             descriptor_index,
		code_or_native_function        code,
		exception_handlers_container&& exception_handlers
	) :
		base_type{ access_flags, name_index, descriptor_index },
		code_        { code },
		exception_handlers_{ move(exception_handlers) }
	{}

	code code() const { return code_.get<::code>(); }
	exception_handlers_container& exception_handlers() {
		return exception_handlers_;
	}

	bool is_native() const {
		return access_flags().get(class_file::access_flag::native);
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
#pragma once

#include "class/file/access_flag.hpp"
#include "class/file/constant.hpp"
#include "../native/function/declaration.hpp"

#include <core/span.hpp>
#include <core/meta/elements/one_of.hpp>

struct _class;

struct code : span<uint8, uint32> {
	using base_type = span<uint8, uint32>;

	uint16 max_locals;
	uint16 max_stack;

	code() = default;

	code(span<uint8, uint32> bytes, uint16 max_locals, uint16 max_stack) :
		base_type{ bytes },
		max_locals{ max_locals },
		max_stack{ max_stack }
	{}

};

using code_or_native_function =
	elements::one_of<elements::none, code, native_function&>;

struct method {
private:
	_class&                  class_;
	class_file::access_flags access_flags_;
	uint16                   name_index_;
	uint16                   desc_index_;
	code_or_native_function  code_;
public:

	method(
		_class&                  c,
		class_file::access_flags access_flags,
		uint16                   name_index,
		uint16                   descriptor_index,
		code_or_native_function  code
	) :
		class_       { c },
		access_flags_{ access_flags },
		name_index_  { name_index },
		desc_index_  { descriptor_index },
		code_        { code }
	{}

	_class& _class() const { return class_; }

	inline class_file::constant::utf8 name() const;
	inline class_file::constant::utf8 descriptor() const;

	bool is_native() const {
		return access_flags_.get(class_file::access_flag::native);
	}

	code code() const { return code_.get<::code>(); }

	bool has_native_function() const {
		return code_.is<::native_function&>();
	}

	void native_function(::native_function& f) {
		code_ = f;
	}

	::native_function& native_function() {
		return code_.get<::native_function&>();
	}
};
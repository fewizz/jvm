#pragma once

#include "../native/function/declaration.hpp"
#include "../descriptor_index.hpp"
#include "../name_index.hpp"
#include "class/file/access_flag.hpp"
#include "class/file/constant.hpp"

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

struct method {
private:
	class_file::access_flags access_flags_;
	name_index               name_index_;
	descriptor_index         desc_index_;
	code_or_native_function  code_;
public:

	method(
		class_file::access_flags access_flags,
		name_index               name_index,
		descriptor_index         descriptor_index,
		code_or_native_function  code
	) :
		access_flags_{ access_flags },
		name_index_  { name_index },
		desc_index_  { descriptor_index },
		code_        { code }
	{}

	bool is_native() const {
		return access_flags_.get(class_file::access_flag::native);
	}

	name_index name_index() const {
		return name_index_;
	}

	descriptor_index descriptor_index() const {
		return desc_index_;
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
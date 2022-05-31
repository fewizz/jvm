#pragma once

#include "class/file/access_flag.hpp"
#include "class/file/constant.hpp"

#include <core/meta/elements/one_of.hpp>

struct _class;

struct field {
private:
	_class& class_;
	class_file::access_flags access_flags_;
	uint16 name_index_;
	uint16 desc_index_;

public:
	field(
		_class& c,
		class_file::access_flags access_flags,
		uint16 name_index,
		uint16 descriptor_index
	) :
		class_{ c },
		access_flags_{ access_flags },
		name_index_{ name_index },
		desc_index_{ descriptor_index }
	{}

	_class& _class() const { return class_; }
	class_file::access_flags access_flags() const { return access_flags_; }

	inline class_file::constant::utf8 name() const;
	inline class_file::constant::utf8 descriptor() const;

	inline bool is_static() const {
		return access_flags_.get(class_file::access_flag::_static);
	}
};

#include "class.hpp"

class_file::constant::utf8
field::name() const { return class_.utf8_constant(name_index_); }

class_file::constant::utf8
field::descriptor() const { return class_.utf8_constant(desc_index_); }
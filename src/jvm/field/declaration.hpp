#pragma once

#include "../name_index.hpp"
#include "../descriptor_index.hpp"
#include "class/file/access_flag.hpp"
#include "class/file/constant.hpp"

#include <core/meta/elements/one_of.hpp>

struct _class;

struct field {
private:
	_class&                  class_;
	class_file::access_flags access_flags_;
	name_index               name_index_;
	descriptor_index         desc_index_;

public:
	field(
		_class&                  c,
		class_file::access_flags access_flags,
		name_index               name_index,
		descriptor_index         descriptor_index
	) :
		class_       { c                },
		access_flags_{ access_flags     },
		name_index_  { name_index       },
		desc_index_  { descriptor_index }
	{}

	_class& _class() const { return class_; }
	class_file::access_flags access_flags() const { return access_flags_; }

	inline class_file::constant::utf8 name() const;
	inline class_file::constant::utf8 descriptor() const;

	inline bool is_static() const {
		return access_flags_.get(class_file::access_flag::_static);
	}
};
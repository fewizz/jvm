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

	class_file::access_flags access_flags() const { return access_flags_; }

	inline class_file::constant::utf8 name() const;
	inline class_file::constant::utf8 descriptor() const;
};

using field_value = elements::one_of<
	bool, int8, uint16, int16, int32, int64,
	float, double
>;

struct static_field : field {
private:
	field_value value_;
public:
	static_field(field f, field_value val):
		field{ f }, value_{ val }
	{};

	auto& value() { return value_; }
};

#include "class.hpp"

class_file::constant::utf8
field::name() const { return class_.utf8_constant(name_index_); }

class_file::constant::utf8
field::descriptor() const { return class_.utf8_constant(desc_index_); }
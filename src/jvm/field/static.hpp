#pragma once

#include "declaration.hpp"
#include "value.hpp"

struct static_field : field {
private:
	field_value value_;
public:
	static_field(field f, field_value val):
		field{ f }, value_{ val }
	{};

	auto& value() { return value_; }
};
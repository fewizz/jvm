#pragma once

#include "decl/class.hpp"
#include "decl/array.hpp"
#include "decl/object/create.hpp"

static optional<_class&> string_class{};
inline instance_field_index string_value_index{};

template<typename Handler>
inline void for_each_string_codepoint(object& str, Handler&& handler);

static inline reference create_string(span<uint16> data);

template<basic_range String>
static reference create_string_from_utf8(String&& str_utf8);
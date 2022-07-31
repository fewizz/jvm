#pragma once

#include "../class.hpp"
#include <core/range_of_value_type_same_as.hpp>

template<range_of<uint8> BytesRange>
static _class& define_class(BytesRange&& bytes);
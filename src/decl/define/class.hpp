#pragma once

#include "../class.hpp"
#include <range.hpp>

template<range_of<uint8> BytesRange>
static _class& define_class(BytesRange&& bytes);
#pragma once

#include "declaration.hpp"

#include "../class/declaration.hpp"

class_file::constant::utf8
field::name() const { return class_.utf8_constant(name_index_); }

class_file::constant::utf8
field::descriptor() const { return class_.utf8_constant(desc_index_); }
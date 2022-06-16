#pragma once

#include "declaration.hpp"

#include "../class/declaration.hpp"

class_file::constant::utf8
method::name() const { return class_.utf8_constant(name_index_); }

class_file::constant::utf8
method::descriptor() const { return class_.utf8_constant(desc_index_); }
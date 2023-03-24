#pragma once

#include "../class.hpp"
#include <range.hpp>

static _class& define_class(
	posix::memory_for_range_of<uint8> bytes,
	reference definig_loader
);
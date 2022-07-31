#pragma once

#include <core/integer.hpp>
#include <core/span.hpp>

struct code : span<uint8, uint32> {
	using base_type = span<uint8, uint32>;

	uint16 max_stack;
	uint16 max_locals;

	code() = default;

	code(span<uint8, uint32> bytes, uint16 max_stack, uint16 max_locals) :
		base_type{ bytes },
		max_stack{ max_stack },
		max_locals{ max_locals }
	{}

};
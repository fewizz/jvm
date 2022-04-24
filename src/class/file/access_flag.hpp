#pragma once

#include <core/flag_enum.hpp>

namespace class_file {

	enum class access_flag {
		_public    = 0x0001,
		_final     = 0x0010,
		super      = 0x0020,
		interface  = 0x0200,
		abstract   = 0x0400,
		synthetic  = 0x1000,
		annotation = 0x2000,
		_enum      = 0x4000,
		_module    = 0x8000
	};

	using access_flags = flag_enum<access_flag>;

}
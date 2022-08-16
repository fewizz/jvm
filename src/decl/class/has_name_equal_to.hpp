#pragma once

#include <range.hpp>

template<basic_range Name>
struct has_name_equal_to {
	Name name;

	bool operator () (auto& e) const {
		return
			range{ e.name() }.equals_to(name);
	}

};

template<basic_range Name>
has_name_equal_to(Name&&)
	-> has_name_equal_to<Name>;
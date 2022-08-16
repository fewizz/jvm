#pragma once

#include <range.hpp>

template<basic_range Name, basic_range Descritptor>
struct has_name_and_desriptor_equal_to {
	Name name; Descritptor descriptor;

	bool operator () (auto& e) const {
		return
			range{ e.name() }.equals_to(name) &&
			range{ e.descriptor() }.equals_to(descriptor);
	}

};

template<basic_range Name, basic_range Descritptor>
has_name_and_desriptor_equal_to(Name&&, Descritptor&&)
	-> has_name_and_desriptor_equal_to<Name, Descritptor>;
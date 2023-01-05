#pragma once

#include <range.hpp>

template<basic_range Name, basic_range Descritptor>
struct has_name_and_descriptor_equal_to {
	Name name; Descritptor descriptor;

	bool operator () (auto& e) const {
		return
			range{ e.name() }.has_equal_size_and_elements(name) &&
			range{ e.descriptor() }.has_equal_size_and_elements(descriptor);
	}

};

template<basic_range Name, basic_range Descritptor>
has_name_and_descriptor_equal_to(Name&&, Descritptor&&)
	-> has_name_and_descriptor_equal_to<Name, Descritptor>;
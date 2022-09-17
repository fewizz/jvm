#pragma once

#include <range.hpp>

template<basic_range Name, basic_range Descritptor>
struct has_name_and_descriptor_equal_to {
	Name name; Descritptor descriptor;

	bool operator () (auto& e) const {
		return
			range{ e.name() }.have_elements_equal_to(name) &&
			range{ e.descriptor() }.have_elements_equal_to(descriptor);
	}

};

template<basic_range Name, basic_range Descritptor>
has_name_and_descriptor_equal_to(Name&&, Descritptor&&)
	-> has_name_and_descriptor_equal_to<Name, Descritptor>;
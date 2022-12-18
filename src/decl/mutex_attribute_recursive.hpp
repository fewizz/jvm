#pragma once

#include <posix/thread.hpp>

inline body<posix::mutex_attribute>& get_mutex_attribute_recursive() {
	static body<posix::mutex_attribute>	attrib = []() {
		body<posix::mutex_attribute> a = posix::create_mutex_attribute();
		a->set_type(posix::mutex_attribute_type::recursive);
		return a;
	}();
	return attrib;
}
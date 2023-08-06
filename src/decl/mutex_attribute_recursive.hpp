#pragma once

#include <posix/thread.hpp>

static body<posix::mutex_attribute> mutex_attribute_recursive {
	[] {
		body<posix::mutex_attribute> a = posix::create_mutex_attribute();
		a->set_type(posix::mutex_attribute_type::recursive);
		return a;
	}()
};
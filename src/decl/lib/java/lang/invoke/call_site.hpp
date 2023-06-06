#pragma once

#include "decl/object.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"

namespace j {

struct call_site : object {
	static inline optional<instance_method&> get_target_instance_method;

	using object::object;

	reference get_target() {
		stack.emplace_back(*this);
		try_invoke_virtual_resolved_non_polymorphic(
			get_target_instance_method.get()
		).if_has_value([](auto){ posix::abort(); });

		return stack.pop_back<reference>();
	}

};

}
#pragma once

#include "decl/object.hpp"
#include "decl/classes.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/invoke/call_site.hpp"

namespace jvm {

struct constant_call_site : j::call_site {
	using j::call_site::call_site;

	static inline expected<reference, reference> try_create(
		j::method_handle& mh
	) {
		::c& c = classes.load_class_by_bootstrap_class_loader(
			c_string{ u8"java/lang/invoke/ConstantCallSite" }
		);

		instance_method& constructor = c.declared_instance_methods().find(
			c_string{ u8"<init>" },
			c_string{ u8"(Ljava/lang/invoke/MethodHandle;)V" }
		);

		return try_create_object(constructor, mh);
	}

};

}
#include "decl/lib/jvm/mh/varargs_collector_adapter.hpp"

#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/class.hpp"
#include "decl/classes.hpp"

static void init_jvm_mh_varargs_collector_adapter() {
	c& c = classes.load_class_by_bootstrap_class_loader(
		u8"jvm/mh/VarargsCollectorAdapter"s
	);

	c.declared_instance_methods().find(
		u8"invokeExactPtr"s, u8"()V"s
	).native_function(
		(void*)+[](
			[[maybe_unused]] j::method_handle& ths
		) -> optional<reference> {
			
		}
	);
}
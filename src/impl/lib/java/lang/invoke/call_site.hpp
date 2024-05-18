#include "decl/lib/java/lang/invoke/call_site.hpp"

#include "decl/classes.hpp"

static void init_java_lang_invoke_call_site() {
	c& c = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/invoke/CallSite"sv
	);

	j::call_site::get_target_instance_method
		= c.instance_methods().find(
			u8"getTarget"sv,
			u8"()Ljava/lang/invoke/MethodHandle;"sv
		);
}
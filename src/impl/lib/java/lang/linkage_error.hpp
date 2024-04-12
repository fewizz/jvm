#include "decl/lib/java/lang/linkage_error.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"

#include <optional.hpp>

[[nodiscard]] inline expected<reference, reference>
try_create_linkage_error() {
	expected<c&, reference> possible_c
		= classes.try_load_class_by_bootstrap_class_loader(
			u8"java/lang/LinkageError"s
		);
	
	if(possible_c.is_unexpected()) {
		return unexpected{ possible_c.move_unexpected() };
	}

	c& c = possible_c.get_expected();
	instance_method& constructor = c.instance_methods().find(
		u8"<init>"s, u8"()V"s
	);

	return try_create_object(constructor);
}
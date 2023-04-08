#include "decl/lib/java/lang/incompatible_class_change_error.hpp"

#include "decl/classes.hpp"
#include "decl/object.hpp"

#include <optional.hpp>

[[nodiscard]] inline expected<reference, reference>
try_create_incompatible_class_change_error() {
	expected<_class&, reference> possible_c
		= classes.try_load_class_by_bootstrap_class_loader(
			c_string{"java/lanf/IncompatibleClassChangeError"}
		);
	
	if(possible_c.is_unexpected()) {
		return unexpected{ move(possible_c.get_unexpected()) };
	}

	_class& c = possible_c.get_expected();
	method& constructor = c.instance_methods().find(
		c_string{"<init>"}, c_string{"()V"}
	);

	return try_create_object(constructor);
}
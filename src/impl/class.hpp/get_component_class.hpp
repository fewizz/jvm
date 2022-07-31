#include "class.hpp"
#include "classes.hpp"

#include <core/to_range.hpp>

inline _class& _class::get_component_class() {
	// component class for primitives are passed by constructor,
	// no special handling needed
	if(!component_class_.has_value()) {
		if(!is_array_class()) {
			fputs("asking component class of non-array class", stderr);
			abort();
		}
		auto n = name();
		// skip [L and ;
		auto component_name = to_range(n.begin() + 2, n.begin() + n.size() - 1);
		component_class_ = classes.find_or_load(component_name);
		component_class_->array_class_ = *this;
	}
	return component_class_.value();
}
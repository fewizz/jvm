#include "class.hpp"
#include "classes.hpp"

#include <iterator_and_sentinel.hpp>

inline _class& _class::get_component_class() {
	// component class for primitives are passed by constructor,
	// no special handling needed
	if(!component_class_.has_value()) {
		if(!is_array()) {
			fputs("asking component class of non-array class", stderr);
			abort();
		}
		auto n = name();
		// skip [L and ;
		auto component_name = iterator_and_sentinel {
			n.iterator() + 2, n.iterator() + n.size() - 1
		}.to_range();
		component_class_ = classes.find_or_load(component_name);
		component_class_->array_class_ = *this;
	}
	return component_class_.value();
}
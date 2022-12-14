#include "class.hpp"
#include "classes.hpp"

#include <range.hpp>

inline _class& _class::get_array_class() {
	// array class for primitives are passed by constructor,
	// no special handling needed
	if(!array_class_.has_value()) {
		auto array_class_name = ranges {
			array{ '[' }, descriptor()
		}.concat_view();
		array_class_ = classes.find_or_load(array_class_name);
		array_class_->component_class_ = *this;
	}
	return array_class_.get();
}
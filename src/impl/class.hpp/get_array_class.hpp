#include "class.hpp"
#include "classes.hpp"

#include <core/concat.hpp>

inline _class& _class::get_array_class() {
	// array class for primitives are passed by constructor,
	// no special handling needed
	if(!array_class_.has_value()) {
		// case when current class is already an array of some dimensionality
		// for example, array of [Ljava/lang/String; is [[Ljava/lang/String
		if(is_array_class()) {
			concat_view array_class_name {
				c_string{ "[" }, name()
			};
			array_class_ = classes.find_or_load(array_class_name);
		}
		else {
			concat_view array_class_name {
				c_string{ "[L" }, name(), c_string{ ";" }
			};
			array_class_ = classes.find_or_load(array_class_name);
		}
		array_class_->component_class_ = *this;
	}
	return array_class_.value();
}
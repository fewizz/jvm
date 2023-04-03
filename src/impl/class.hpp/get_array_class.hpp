#include "class.hpp"
#include "classes.hpp"

#include <range.hpp>

inline _class& _class::get_array_class() {
	mutex_->lock();
	if(!array_class_.has_value()) {

		if(is_primitive()) {
			auto array_class_name = ranges {
				array{ '[' }, descriptor()
			}.concat_view();

			array_class_ = classes.load_array_class(
				array_class_name, defining_loader_
			);
		}
		else {
			auto array_class_name = ranges {
				array{ '[' }, descriptor(), array{ ';' }
			}.concat_view();

			array_class_ = classes.load_array_class(
				array_class_name, defining_loader_
			);
		}

		array_class_->component_class_ = *this;
	}
	mutex_->unlock();
	return array_class_.get();
}
#include "class.hpp"
#include "classes.hpp"

#include <range.hpp>

inline c& c::get_array_class() {
	mutex_->lock();
	if(!array_class_.has_value()) {

		auto array_class_name = ranges {
			array{ u8'[' }, descriptor()
		}.concat_view();

		array_class_ = classes.load_array_class(
			array_class_name, defining_loader_.object_ptr()
		);

		array_class_->component_class_ = *this;
	}
	mutex_->unlock();
	return array_class_.get();
}
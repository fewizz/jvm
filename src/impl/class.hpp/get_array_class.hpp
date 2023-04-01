#include "class.hpp"
#include "classes.hpp"

#include <range.hpp>

inline _class& _class::get_array_class() {
	// array class for primitives are passed by constructor,
	// no special handling needed
	posix::abort(); // TODO
	/*mutex_->lock();
	if(!array_class_.has_value()) {
		auto array_class_name = ranges {
			array{ '[' }, descriptor(), array{ ';' }
		}.concat_view();
		array_class_ = classes.find_or_load_by_bootstrap_classloader(array_class_name);
		array_class_->component_class_ = *this;
	}
	mutex_->unlock();
	return array_class_.get();*/
}
#include "class.hpp"
#include "lib/java/lang/class.hpp"

inline reference _class::instance() {
	if(instance_.is_null()) {
		instance_ = create_object(class_class.value());
		instance_.object().values()[class_ptr_field_index] = (int64) this;
	}
	return instance_;
}
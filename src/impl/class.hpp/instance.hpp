#include "decl/class.hpp"

#include "lib/java/lang/class.hpp"
#include "object.hpp"
#include "object.hpp"

inline reference _class::instance() {
	if(instance_.is_null()) {
		instance_ = create_object(class_class.value());
		instance_->set<int64>(class_ptr_field_position, (int64) this);
	}
	return instance_;
}
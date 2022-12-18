#include "decl/class.hpp"

#include "lib/java/lang/class.hpp"
#include "object.hpp"
#include "object.hpp"

inline reference _class::instance() {
	mutex_->lock();
	on_scope_exit unlock {[&] {
		mutex_->unlock();
	}};

	if(instance_.is_null()) {
		instance_ = create_object(class_class.get());
		instance_->set<int64>(class_ptr_field_position, (int64) this);
	}
	return instance_;
}
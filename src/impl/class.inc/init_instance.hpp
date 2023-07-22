#include "decl/class.hpp"

#include "decl/lib/java/lang/class.hpp"
#include "decl/object.hpp"

#include <on_scope_exit.hpp>

void c::init_instance() {
	mutex_->lock();
	on_scope_exit unlock {[&] {
		mutex_->unlock();
	}};

	if(instance_.is_null()) {
		expected<reference, reference> possible_c
			= try_create_object(class_class.get());
		if(possible_c.is_unexpected()) {
			print::err("couldn't create ", name(), "class instance\n");
			posix::abort();
		}

		instance_ = possible_c.move_expected();
		instance_->set<int64>(class_ptr_field_position, (int64) this);
	}
}
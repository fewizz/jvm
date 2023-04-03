#include "class.hpp"
#include "classes.hpp"

#include <iterator_and_sentinel.hpp>

inline _class& _class::get_component_class() {
	posix::abort();

	// TODO
	// component class for primitives are passed by constructor,
	// no special handling needed
	/*mutex_->lock();
	on_scope_exit unlock {[&] {
		mutex_->unlock();
	}};

	if(!component_class_.has_value()) {
		if(!is_array()) {
			posix::abort();
		}
		auto n = name();
		bool component_is_reference = n[1] == 'L';
		auto component_name =
			component_is_reference ?
			// skip [L and ;
			iterator_and_sentinel {
				n.iterator() + 2, n.iterator() + n.size() - 1
			}.as_range() :
			// skip [
			iterator_and_sentinel {
				n.iterator() + 1, n.sentinel()
			}.as_range();
		component_class_ = load_class_by_bootstrap_class_loader(component_name);
		component_class_->array_class_ = *this;
	}
	return component_class_.get();*/
}
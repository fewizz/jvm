#include "class.hpp"
#include "classes.hpp"

#include <iterator_and_sentinel.hpp>

inline _class& _class::get_component_class() {
	// component class for primitives are passed by constructor,
	// no special handling needed
	if(!component_class_.has_value()) {
		if(!is_array()) {
			//fputs("asking component class of non-array class", stderr);
			abort();
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
		component_class_ = classes.find_or_load(component_name);
		component_class_->array_class_ = *this;
	}
	return component_class_.get();
}
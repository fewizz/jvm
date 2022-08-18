#include "decl/array.hpp"

#include "object/create.hpp"

template<typename Type>
static inline reference create_array_by_class(
	_class& array_class, int32 length
) {
	reference ref = create_object(array_class);
	array_length(ref.object(), length);
	reference* data = (reference*) allocate_zeroed_for<Type>(
		length
	).elements_ptr();
	array_data(ref.object(), data);
	return ref;
}

static inline reference create_array_of(
	_class& element_class, int32 length
) {
	_class& array_class = element_class.get_array_class();
	return create_array_by_class<reference>(array_class, length);
}
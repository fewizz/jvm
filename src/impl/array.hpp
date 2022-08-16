#include "decl/array.hpp"

#include "object/create.hpp"

static inline reference create_array_by_class(
	_class& array_class, nuint element_size, int32 length
) {
	reference* data = (reference*) allocate_zeroed(
		element_size * length
	).elements_ptr();
	reference ref = create_object(array_class);
	array_length(ref.object(), length);
	array_data(ref.object(), data);
	return ref;
}

static inline reference create_array_of(
	_class& element_class, int32 length
) {
	_class& array_class = element_class.get_array_class();
	return create_array_by_class(array_class, sizeof(reference), length);
}
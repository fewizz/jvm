#include "classes.hpp"
#include "define/array_class.hpp"

#include "lib/java/lang/object.hpp"

#include <range.hpp>
#include <array.hpp>

template<basic_range Name>
static inline _class& define_primitive_class(Name&& name) {
	memory_span data { allocate(name.size()) };
	range{ name }.copy_to(data);

	return classes.emplace_back(
		constants{}, bootstrap_methods{},
		data,
		class_file::access_flags{ class_file::access_flag::_public },
		this_class_name{ data.cast<const char>() }, object_class,
		declared_interfaces{},
		declared_fields{},
		declared_methods{},
		is_array_class{ false },
		is_primitive_class{ true }
	);
}

template<basic_range Name>
static inline _class& define_primitive_and_its_array_classes(
	Name&& name, char ch
) {
	_class& component_class = define_primitive_class(name);

	array<char, 2> array_class_name{ '[', ch };
	_class& array_class = define_array_class(array_class_name);

	array_class.component_class(component_class);
	component_class.array_class(array_class);

	return component_class;
}
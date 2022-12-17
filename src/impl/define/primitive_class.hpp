#include "classes.hpp"
#include "define/array_class.hpp"

#include "lib/java/lang/object.hpp"

#include <range.hpp>
#include <array.hpp>

template<basic_range Name>
static inline _class& define_primitive_class(Name&& name, char ch) {
	auto data = posix::allocate_memory_for<uint8>(name.size());
	span<char> data_span{ (char*) data.iterator(), data.size() };
	range{ name }.copy_to(data_span);

	auto descriptor = posix::allocate_memory_for<uint8>(1);
	descriptor[0].construct((uint8)ch);

	return classes.emplace_back(
		constants{}, bootstrap_methods{},
		move(data),
		class_file::access_flags{ class_file::access_flag::_public },
		this_class_name{ data_span },
		move(descriptor),
		object_class,
		posix::memory_for_range_of<_class*>{},
		posix::memory_for_range_of<field>{},
		posix::memory_for_range_of<method>{},
		is_array_class{ false },
		is_primitive_class{ true }
	);
}

template<basic_range Name>
static inline _class& define_primitive_and_its_array_classes(
	Name&& name, char ch
) {
	_class& component_class = define_primitive_class(name, ch);

	array<char, 2> array_class_name{ '[', ch };
	_class& array_class = define_array_class(array_class_name);

	array_class.component_class(component_class);
	component_class.array_class(array_class);

	return component_class;
}
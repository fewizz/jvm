#include "classes.hpp"
#include "define/array_class.hpp"

#include <core/range.hpp>
#include <core/copy.hpp>
#include <core/array.hpp>

template<range Name>
static inline _class& define_primitive_class(Name&& name) {
	c_string base_class{ "java/lang/Object" };

	const_pool const_pool{ 4 };

	span<uint8> data {
		default_allocator{}.allocate(name.size()), (uint16) name.size()
	};
	copy{ name }.to(data);

	// 1
	const_pool.emplace_back(
		class_file::constant::utf8 {
			(char*) data.data(), (uint16) name.size()
		}
	);

	// 2
	const_pool.emplace_back(
		class_file::constant::_class{ class_file::constant::name_index{ 1 } }
	);

	// 3
	const_pool.emplace_back(class_file::constant::utf8 {
		base_class.data(), (uint16) base_class.size()
	});

	// 4
	const_pool.emplace_back(
		class_file::constant::_class{ class_file::constant::name_index{ 3 } }
	);

	return classes.emplace_back(
		move(const_pool), data,
		class_file::access_flags{ class_file::access_flag::_public },
		this_class_index{ 2 }, super_class_index{ 4 },
		interfaces_indices_container{},
		instance_fields_container{},
		static_fields_container{},
		methods_container{},
		is_array_class{ false },
		is_primitive_class{ true }
	);
}

template<range Name>
static inline _class& define_primitive_class_and_its_array(
	Name&& name, char ch
) {
	_class& component_class = define_primitive_class(name);

	array<char, 2> array_class_name{ '[', ch };
	_class& array_class = define_array_class(array_class_name);

	array_class.unsafe_set_component_class(component_class);
	component_class.unsafe_set_array_class(array_class);

	return component_class;
}
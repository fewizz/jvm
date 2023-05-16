#pragma once

#include "decl/classes.hpp"

/* The following steps are used to create the array class C denoted by the name
   N in association with the class loader L. L may be either the bootstrap class
   loader or a user-defined class loader. */
template<basic_range Name>
expected<c&, reference> classes::try_load_array_class(
	Name&& name, object_of<jl::c_loader>* l
) {
	mutex_->lock();
	on_scope_exit unlock_classes_mutex { [&] {
		mutex_->unlock();
	}};

	/* First, the Java Virtual Machine determines whether L has already been
	   recorded as an initiating loader of an array class with the same
	   component type as N. If so, this class is C, and no array class creation
	   is necessary. */
	
	optional<c&> loaded_class
		= try_find_class_which_loading_was_initiated_by(name, l);

	if(loaded_class.has_value()) {
		return loaded_class.get();
	}

	if(info) {
		tabs();
		print::out("loading array class ", name, "\n");
	}

	/* Otherwise, the following steps are performed to create C: */

	/* 1. If the component type is a reference type, the algorithm of this
	      section (§5.3) is applied recursively using L in order to load and
	      thereby create the component type of C. */
	utf8::unit component_type = name[1];

	bool component_is_array = component_type == '[';
	bool component_is_class = component_type == 'L';

	bool component_is_reference = component_is_array || component_is_class;

	expected<c&, reference> possible_component_class = [&]()
	-> expected<c&, reference> {
		if(component_is_reference) {
			auto iter = name.iterator() + 1;
			nuint size = name.size() - 1;

			if(component_is_class) {
				++iter; // skip L
				--size;

				--size; // skip ';'
			}

			expected<c&, reference> possible_component_class
				= iterator_and_sentinel {
					iter, iter + size
				}.as_range().view_copied_elements_on_stack([&](auto cn) {
					return try_load_class(cn, l);
				});

			return possible_component_class;
		}

		switch (component_type) {
			case 'Z' : return bool_class.get();
			case 'B' : return byte_class.get();
			case 'C' : return char_class.get();
			case 'S' : return short_class.get();
			case 'I' : return int_class.get();
			case 'F' : return float_class.get();
			case 'J' : return long_class.get();
			case 'D' : return double_class.get();
			default: posix::abort();
		}
	}();

	if(possible_component_class.is_unexpected()) {
		return unexpected{ possible_component_class.move_unexpected() };
	}

	c& component_class = possible_component_class.get_expected();

	/* 2. The Java Virtual Machine creates a new array class with the indicated
	      component type and number of dimensions. */
	/*    If the component type is a reference type, the Java Virtual Machine
	      marks C to have the defining loader of the component type as its
	      defining loader. Otherwise, the Java Virtual Machine marks C to have
	      the bootstrap class loader as its defining loader.*/

	object_of<jl::c_loader>* defining_class_loader =
		(object_of<jl::c_loader>*)
		component_class.defining_loader().object_ptr();

	optional<class_and_initiating_loaders&> c_and_l =
		try_find_first_satisfying([&](class_and_initiating_loaders& c_and_il) {
			c& c = c_and_il.class_;
			return
				c.name().has_equal_size_and_elements(name) &&
				c.defining_loader().object_ptr() == defining_class_loader;
		}
	);

	c& c = c_and_l.has_value() ?
		c_and_l.get().class_ :
		define_array_class(name, defining_class_loader);

	/*    In any case, the Java Virtual Machine then records that L is an
	      initiating loader for C (§5.3.4). */
	
	if(component_class.defining_loader().object_ptr() != l) {
		mark_class_loader_as_initiating_for_class(c, l);
	}

	/*    If the component type is a reference type, the accessibility of the
	      array class is determined by the accessibility of its component type
	      (§5.4.4). Otherwise, the array class is accessible to all classes and
	      interfaces. */
	// TODO

	return c;
}
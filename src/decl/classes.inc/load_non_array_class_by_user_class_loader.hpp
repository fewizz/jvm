#pragma once

#include "decl/classes.hpp"

/* The process of loading and creating the nonarray class or interface C denoted
   by N using a user-defined class loader L is as follows. */
template<basic_range Name>
expected<c&, reference>
classes::try_load_non_array_class_by_user_class_loader(
	Name&& name, o<jl::c_loader>* l
) {
	if(l == nullptr) {
		posix::abort();
	}
	c& l_c = l->c();

	mutex_->lock();
	on_scope_exit unlock_classes_mutex { [&] {
		mutex_->unlock();
	}};

	/* First, the Java Virtual Machine determines whether L has already been
	   recorded as an initiating loader of a class or interface denoted by N. If
	   so, this class or interface is C, and no class loading or creation is
	   necessary. */
	optional<c&> loaded_class
		= try_find_class_which_loading_was_initiated_by(name, l);
	
	if(loaded_class.has_value()) {
		return loaded_class.get();
	}

	if(info) {
		tabs();
		print::out("loading non-array class ");

		range{name}.view_copied_elements_on_stack([&](auto on_stack) {
			print::out(on_stack);
		});

		print::out(" by user class-loader ", l_c.name(), "\n");
	}

	/* Otherwise, the Java Virtual Machine invokes the loadClass method of class
	   ClassLoader on L, passing the name N of a class or interface. */
	expected<reference, reference> possible_name_ref
		= try_create_string_from_utf8(name);
	if(possible_name_ref.is_unexpected()) {
		return unexpected{ possible_name_ref.move_unexpected() };
	}
	reference name_ref = possible_name_ref.move_expected();
	method& load_method = l_c[class_loader_load_class_method_index];

	optional<reference> possible_exception
		= try_execute(load_method, reference{*l}, name_ref);
	if(possible_exception.has_value()) {
		return possible_exception.move();
	}

	/* If the invocation of loadClass on L has a result, then: */
	/* If the result is null, or the result is a class or interface with a name
	   other than N, then the result is discarded, and the process of loading
	   and creation fails with a NoClassDefFoundError. */
	reference resulting_class_ref = stack.pop_back<reference>();
	if(resulting_class_ref.is_null()) {
		posix::abort(); // TODO
	}

	c& c = class_from_class_instance(resulting_class_ref);

	if(!c.name().has_equal_size_and_elements(name)) {
		print::err(
			"loaded class name is ", c.name(),
			", but should be ", name, "\n"
		);
		posix::abort(); // TODO
	}

	/* Otherwise, the result is the created class or interface C. The Java
	   Virtual Machine records that L is an initiating loader of C (§5.3.4).
	   The process of loading and creating C succeeds. */
	if(c.defining_loader().object_ptr() != l) {
		mark_class_loader_as_initiating_for_class(c, l);
	}
	return c;
}

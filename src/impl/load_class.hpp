#include "decl/load_class.hpp"

#include "decl/class.hpp"
#include "decl/classes.hpp"
#include "decl/define/class.hpp"
#include "decl/executable_path.hpp"
#include "decl/execution/info.hpp"
#include "decl/execute.hpp"
#include "decl/thrown.hpp"
#include "decl/primitives.hpp"
#include "decl/lib/java/lang/class_loader.hpp"
#include "decl/lib/java/lang/class.hpp"

#include <array.hpp>
#include <range.hpp>
#include <ranges.hpp>
#include <c_string.hpp>
#include <optional.hpp>
#include <on_scope_exit.hpp>
#include <expression_of_type.hpp>

#include <print/print.hpp>
#include <posix/memory.hpp>

template<basic_range Name>
inline _class& load_class(Name&& name, reference class_loader);

template<basic_range Name, basic_range RootPath>
inline optional<posix::memory_for_range_of<unsigned char>>
try_load_class_file_data_at(RootPath&& root_path, Name&& name) {
	auto null_terminated = ranges {
		root_path, array{'/'},
		name, c_string{ ".class" }, array{ '\0' }
	}.concat_view();

	expected<body<posix::file>, posix::error> possible_file
		= null_terminated.view_copied_elements_on_stack(
			[&](span<const char> on_stack) {
				return posix::try_open_file(
					c_string{ on_stack.iterator() },
					posix::file_access_mode::binary_read
				);
			}
		);

	if(possible_file.is_unexpected()) {
		return {};
	}

	body<posix::file> f = move(possible_file.get_expected());

	nuint size = f->set_offset_to_end();
	f->set_offset(0);

	posix::memory_for_range_of<unsigned char> data
		= posix::allocate_memory_for<unsigned char>(size);

	nuint read_total = 0;
	while(read_total < size) {
		nuint read = f->read_to(
			span{data.iterator() + read_total, size - read_total}
		);
		if(read == 0) break;
		read_total += read;
	}

	return { move(data) };
}

/* The process of loading and creating the nonarray class or interface C denoted
by N using the bootstrap class loader is as follows. */
template<basic_range Name>
inline _class&
load_non_array_class_by_bootstrap_class_loader(
	Name&& name
) {
	auto& m = classes.mutex();
	m->lock();
	on_scope_exit unlock_classes_mutex { [&] {
		m->unlock();
	}};

	/* First, the Java Virtual Machine determines whether the bootstrap class
	   loader has already been recorded as an initiating loader of a class or
	   interface denoted by N. If so, this class or interface is C, and no class
	   loading or creation is necessary. */
	class_and_initiating_loaders& c_and_il
		= classes.try_find_class_and_initiating_loaders(name);

	if(c_and_il.loader_is_recorded_as_initiating(reference{})) {
		return c_and_il._class;
	}

	if(info) {
		tabs();
		print::out("loading non-array class ");

		range{name}.view_copied_elements_on_stack([&](auto on_stack) {
			print::out(on_stack);
		});

		print::out(" by bootstrap class-loader\n");
	}

	/* Otherwise, the Java Virtual Machine passes the argument N to an
	   invocation of a method on the bootstrap class loader. To load C, the
	   bootstrap class loader locates a purported representation of C in a
	   platform-dependent manner, then asks the Java Virtual Machine to derive a
	   class or interface C denoted by N from the purported representation using
	   the bootstrap class loader, and then to create C, via the algorithm of
	   §5.3.5. */

	optional<posix::memory_for_range_of<unsigned char>> possible_data =
		ranges{ lib_path.get(), c_string{ "/java.base"} }.concat_view()
		.view_copied_elements_on_stack([&](auto root_path_on_stack) {
			return try_load_class_file_data_at(
				root_path_on_stack,
				name
			);
		});

	if(!possible_data.has_value()) {
		print::err(
			"couldn't find class file ", name,
			" by bootstrap class-loader\n"
		);
		posix::abort();
	}

	auto data = move(possible_data.get());

	define_class(move(data), reference{});
}

/* The process of loading and creating the nonarray class or interface C denoted
   by N using a user-defined class loader L is as follows. */
template<basic_range Name>
inline _class& load_non_array_class_by_user_class_loader(
	Name&& name, reference l_ref
) {
	_class& l_c = l_ref._class();

	auto& m = classes.mutex();
	m->lock();
	on_scope_exit unlock_classes_mutex { [&] {
		m->unlock();
	}};

	/* First, the Java Virtual Machine determines whether L has already been
	   recorded as an initiating loader of a class or interface denoted by N. If
	   so, this class or interface is C, and no class loading or creation is
	   necessary. */
	class_and_initiating_loaders& c_and_il
		= classes.try_find_class_and_initiating_loaders(name);
	
	if(c_and_il.loader_is_recorded_as_initiating(l_ref)) {
		return c_and_il._class;
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
	reference name_ref = create_string_from_utf8(name);
	method& load_method = l_c[class_loader_load_class_method_index];
	execute(load_method, l_ref, name_ref);
	if(!thrown.is_null()) {
		print::err(
			"exception was thrown while "
			"loading class by user class-loader\n"
		);
		posix::abort();
	}

	/* If the invocation of loadClass on L has a result, then: */
	/* If the result is null, or the result is a class or interface with a name
	   other than N, then the result is discarded, and the process of loading
	   and creation fails with a NoClassDefFoundError. */
	reference resulting_class_ref = stack.pop_back<reference>();
	if(resulting_class_ref.is_null()) {
		posix::abort(); // TODO
	}

	_class& c = class_from_class_instance(resulting_class_ref);

	if(!c.name().has_equal_size_and_elements(name)) {
		posix::abort(); // TODO
	}

	/* Otherwise, the result is the created class or interface C. The Java
	   Virtual Machine records that L is an initiating loader of C (§5.3.4).
	   The process of loading and creating C succeeds. */
	classes.mark_class_loader_as_initiating_for_class(c, l_ref);
	return c;
}

/* The following steps are used to create the array class C denoted by the name
   N in association with the class loader L. L may be either the bootstrap class
   loader or a user-defined class loader. */
template<basic_range Name>
inline _class&
load_array_class(
	Name&& name, reference l_ref
) {
	auto& m = classes.mutex();
	m->lock();
	on_scope_exit unlock_classes_mutex { [&] {
		m->unlock();
	}};

	/* First, the Java Virtual Machine determines whether L has already been
	   recorded as an initiating loader of an array class with the same
	   component type as N. If so, this class is C, and no array class creation
	   is necessary. */
	
	optional<class_and_initiating_loaders&> possible_c_and_il
		= classes.try_find_class_and_initiating_loaders(name);
	
	if(possible_c_and_il.has_value() && possible_c_and_il.loader_is_recorded_as_initiating(l_ref)) {
		return c_and_il._class;
	}

	/* Otherwise, the following steps are performed to create C: */

	/* 1. If the component type is a reference type, the algorithm of this
	      section (§5.3) is applied recursively using L in order to load and
	      thereby create the component type of C. */
	char component_type = name[1];

	bool component_is_array = component_type == '[';
	bool component_is_class = component_type == 'L';

	bool component_is_reference = component_is_array || component_is_class;

	_class& component_class = [&] {
		if(component_is_reference) {
			auto iter = name.iterator() + 1;
			auto sent = name.sentinel();
			if(component_is_class) {
				--sent; // skip ';'
			}

			auto component_name = iterator_and_sentinel{ iter, sent }.range();

			return load_class(component_name, l_ref);
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

	/* 2. The Java Virtual Machine creates a new array class with the indicated
	      component type and number of dimensions. */
	/*    If the component type is a reference type, the Java Virtual Machine
	      marks C to have the defining loader of the component type as its
	      defining loader. Otherwise, the Java Virtual Machine marks C to have
	      the bootstrap class loader as its defining loader.*/
	_class& c = define_array_class(name, component_class.defining_loader());

	/*    In any case, the Java Virtual Machine then records that L is an
	      initiating loader for C (§5.3.4). */
	
	classes.mark_class_loader_as_initiating_for_class(c, l_ref);

	/*    If the component type is a reference type, the accessibility of the
	      array class is determined by the accessibility of its component type
	      (§5.4.4). Otherwise, the array class is accessible to all classes and
	      interfaces. */
	// TODO

	return c;
}
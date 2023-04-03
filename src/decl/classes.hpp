#pragma once

#include "./mutex_attribute_recursive.hpp"
#include "./class.hpp"

#include "execution/info.hpp"
#include "executable_path.hpp"
#include "lib/java/lang/class_loader.hpp"
#include "lib/java/lang/class.hpp"
#include "execute.hpp"
#include "thrown.hpp"
#include "primitives.hpp"
#include "try_load_class_file_data_at.hpp"

#include <list.hpp>
#include <optional.hpp>
#include <ranges.hpp>

#include <posix/memory.hpp>

struct class_and_initiating_loaders {
	_class _class;
	// TODO make resizeable
	list<posix::memory_for_range_of<reference>> initiating_loaders;

	template<typename... Args>
	class_and_initiating_loaders(
		Args&&... args
	) :
		_class { forward<Args>(args)... },
		initiating_loaders {
			posix::allocate_memory_for<reference>(16)
		}
	{
		initiating_loaders.emplace_back(_class.defining_loader());
	}

	void record_as_initiating(reference ref) {
		if(loader_is_recorded_as_initiating(ref)) {
			print::err("class-loader is already recorded as initiating\n");
			posix::abort();
		}
		initiating_loaders.emplace_back(move(ref));
	}

	bool loader_is_recorded_as_initiating(reference ref) const {
		for(reference& l : initiating_loaders) {
			if(l.object_ptr() == ref.object_ptr()) {
				return true;
			}
		}
		return false;
	}
};

static struct classes :
	private list<posix::memory_for_range_of<class_and_initiating_loaders>>
{
	using base_type = list<
		posix::memory_for_range_of<class_and_initiating_loaders>
	>;
	using base_type::base_type;

	body<posix::mutex> mutex_ = posix::create_mutex(mutex_attribute_recursive);

	~classes() {
		for(class_and_initiating_loaders& c : *this) {
			c._class.destruct_declared_static_fields_values();
		}
	}

	body<posix::mutex>& mutex() { return mutex_; }

	template<basic_range Name>
	_class& load_class_by_bootstrap_class_loader(Name&& name) {
		return range{name}.starts_with('[') ?
			load_array_class(forward<Name>(name), reference{})
			:
			load_non_array_class_by_bootstrap_class_loader(
				forward<Name>(name)
			);
	}

	template<basic_range Name>
	_class& load_non_array_class(Name&& name, reference l_ref) {
		return l_ref.is_null() ?
			load_non_array_class_by_bootstrap_class_loader(
				forward<Name>(name)
			)
			:
			load_non_array_class_by_user_class_loader(
				forward<Name>(name), move(l_ref)
			);
	}

	template<basic_range Name>
	_class& load_class(Name&& name, reference class_loader) {
		if(range{name}.starts_with('[')) {
			return load_array_class(forward<Name>(name), move(class_loader));
		}

		return class_loader.is_null() ?
			load_non_array_class_by_bootstrap_class_loader(
				forward<Name>(name)
			)
			:
			load_non_array_class_by_user_class_loader(
				forward<Name>(name), move(class_loader)
			);
	}

	template<basic_range Name>
	_class& find_or_load(Name&& name) {
		optional<_class&> c = try_find(name);
		return c.set_if_has_no_value([&]() -> _class& {
			return load_class_by_bootstrap_class_loader(
				name
			);
		}).get();
	}

	void mark_class_loader_as_initiating_for_class(_class& c, reference cl) {
		for(auto& c_and_cl : *this) {
			if(&c_and_cl._class == &c) {
				c_and_cl.record_as_initiating(cl);
				return;
			}
		}
		posix::abort();
	}

	template<basic_range Name>
	optional<class_and_initiating_loaders&>
	try_find_class_and_initiating_loaders(Name&& name) {
		optional<class_and_initiating_loaders&> res
			= this->try_find_first_satisfying(
				[&](class_and_initiating_loaders& c_and_l) {
					bool same_name = c_and_l._class.name()
						.has_equal_size_and_elements(name);

					return same_name;
				}
			);
		
		if(res.has_value()) {
			return { res.get() };
		}
		return {};
	}

	template<basic_range Name>
	optional<_class&> try_find_class_which_loading_was_initiated_by(
		Name&& name, reference class_loader
	) {
		optional<class_and_initiating_loaders&> possible_c_and_il
			= this->try_find_first_satisfying(
				[&](class_and_initiating_loaders& c_and_l) {
					bool same_name = c_and_l._class.name()
						.has_equal_size_and_elements(name);
					
					bool cl_is_initiating
						= c_and_l
						.loader_is_recorded_as_initiating(class_loader);

					return same_name && cl_is_initiating;
				}
			);

		if(possible_c_and_il.has_no_value()) {
			return {};
		}

		return possible_c_and_il.get()._class;
	}

	template<typename... Args>
	_class& emplace_back(Args&&... args) {
		class_and_initiating_loaders& c_and_il = base_type::emplace_back(
			forward<Args>(args)...
		);
		return c_and_il._class;
	}

	template<basic_range Name>
	_class& load_non_array_class_by_bootstrap_class_loader(
		Name&& name
	);

	template<basic_range Name>
	_class& load_non_array_class_by_user_class_loader(
		Name&& name, reference l_ref
	);

	template<basic_range Name>
	_class& load_array_class(
		Name&& name, reference l_ref
	);

} classes{ posix::allocate_memory_for<class_and_initiating_loaders>(65536) };

/* The process of loading and creating the nonarray class or interface C denoted
by N using the bootstrap class loader is as follows. */
template<basic_range Name>
_class& classes::load_non_array_class_by_bootstrap_class_loader(
	Name&& name
) {
	auto& m = mutex();
	m->lock();
	on_scope_exit unlock_classes_mutex { [&] {
		m->unlock();
	}};

	/* First, the Java Virtual Machine determines whether the bootstrap class
	   loader has already been recorded as an initiating loader of a class or
	   interface denoted by N. If so, this class or interface is C, and no class
	   loading or creation is necessary. */
	optional<_class&> loaded_class
		= try_find_class_which_loading_was_initiated_by(
			name, reference{}
		);
	
	if(loaded_class.has_value()) {
		return loaded_class.get();
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

	return define_class(forward<Name>(name), move(data), reference{});
}

/* The process of loading and creating the nonarray class or interface C denoted
   by N using a user-defined class loader L is as follows. */
template<basic_range Name>
_class& classes::load_non_array_class_by_user_class_loader(
	Name&& name, reference l_ref
) {
	_class& l_c = l_ref._class();

	auto& m = mutex();
	m->lock();
	on_scope_exit unlock_classes_mutex { [&] {
		m->unlock();
	}};

	/* First, the Java Virtual Machine determines whether L has already been
	   recorded as an initiating loader of a class or interface denoted by N. If
	   so, this class or interface is C, and no class loading or creation is
	   necessary. */
	optional<_class&> loaded_class
		= try_find_class_which_loading_was_initiated_by(name, l_ref);
	
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
	mark_class_loader_as_initiating_for_class(c, l_ref);
	return c;
}

/* The following steps are used to create the array class C denoted by the name
   N in association with the class loader L. L may be either the bootstrap class
   loader or a user-defined class loader. */
template<basic_range Name>
_class& classes::load_array_class(
	Name&& name, reference l_ref
) {
	auto& m = mutex();
	m->lock();
	on_scope_exit unlock_classes_mutex { [&] {
		m->unlock();
	}};

	/* First, the Java Virtual Machine determines whether L has already been
	   recorded as an initiating loader of an array class with the same
	   component type as N. If so, this class is C, and no array class creation
	   is necessary. */
	
	optional<_class&> loaded_class
		= try_find_class_which_loading_was_initiated_by(name, l_ref);
	
	if(loaded_class.has_value()) {
		return loaded_class.get();
	}

	/* Otherwise, the following steps are performed to create C: */

	/* 1. If the component type is a reference type, the algorithm of this
	      section (§5.3) is applied recursively using L in order to load and
	      thereby create the component type of C. */
	char component_type = name[1];

	bool component_is_array = component_type == '[';
	bool component_is_class = component_type == 'L';

	bool component_is_reference = component_is_array || component_is_class;

	_class& component_class = [&]() -> _class& {
		if(component_is_reference) {
			auto iter = name.iterator() + 1;
			nuint size = name.size();

			if(component_is_class) {
				--size; // skip ';'
			}

			_class& component_class = iterator_and_sentinel {
				iter, iter + size
			}.as_range().view_copied_elements_on_stack([&](auto cn) -> _class& {
				return load_class(cn, l_ref);
			});

			return component_class;
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
	
	mark_class_loader_as_initiating_for_class(c, l_ref);

	/*    If the component type is a reference type, the accessibility of the
	      array class is determined by the accessibility of its component type
	      (§5.4.4). Otherwise, the array class is accessible to all classes and
	      interfaces. */
	// TODO

	return c;
}
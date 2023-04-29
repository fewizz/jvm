#pragma once

#include "./mutex_attribute_recursive.hpp"
#include "./class.hpp"

#include "execution/info.hpp"
#include "executable_path.hpp"
#include "lib/java/lang/string.hpp"
#include "lib/java/lang/class_loader.hpp"
#include "lib/java/lang/class.hpp"
#include "lib/java/lang/object.hpp"
#include "lib/java/lang/class_not_found_exception.hpp"
#include "lib/java/lang/linkage_error.hpp"
#include "execute.hpp"
#include "primitives.hpp"
#include "try_load_class_file_data_at.hpp"
#include "class/bootstrap_methods.hpp"
#include "object.hpp"

#include <list.hpp>
#include <optional.hpp>
#include <ranges.hpp>

#include <posix/memory.hpp>
#include <class_file/reader.hpp>

struct class_and_initiating_loaders {
	c class_;
	// TODO make resizeable
	list<posix::memory_for_range_of<reference>> initiating_loaders;

	template<typename... Args>
	class_and_initiating_loaders(
		Args&&... args
	) :
		class_ { forward<Args>(args)... },
		initiating_loaders {
			posix::allocate_memory_for<reference>(16)
		}
	{
		initiating_loaders.emplace_back(class_.defining_loader());
	}

	void record_as_initiating(object* cl) {
		if(loader_is_recorded_as_initiating(cl)) {
			print::err("class-loader is already recorded as initiating\n");
			posix::abort();
		}
		initiating_loaders.emplace_back(*cl);
	}

	bool loader_is_recorded_as_initiating(object* cl) const {
		for(reference& l : initiating_loaders) {
			if(l.object_ptr() == cl) {
				return true;
			}
		}
		return false;
	}
};

static struct classes :
	private list<posix::memory_for_range_of<class_and_initiating_loaders>>
{
private:
	using base_type = list<
		posix::memory_for_range_of<class_and_initiating_loaders>
	>;
	using base_type::base_type;

	body<posix::mutex> mutex_ = posix::create_mutex(mutex_attribute_recursive);
public:

	~classes() {
		for(class_and_initiating_loaders& c : *this) {
			c.class_.destruct_declared_static_fields_values();
		}
	}

private:
	void mark_class_loader_as_initiating_for_class(c& c, object* cl) {
		for(auto& c_and_cl : *this) {
			if(&c_and_cl.class_ == &c) {
				c_and_cl.record_as_initiating(cl);
				return;
			}
		}
		posix::abort();
	}
public:

	template<basic_range Name>
	optional<c&> try_find_class_which_loading_was_initiated_by(
		Name&& name, object* class_loader
	) {
		mutex_->lock();
		on_scope_exit unlock_classes_mutex { [&] {
			mutex_->unlock();
		}};

		optional<class_and_initiating_loaders&> possible_c_and_il
			= this->try_find_first_satisfying(
				[&](class_and_initiating_loaders& c_and_l) {
					bool same_name =
						c_and_l.class_.name().has_equal_size_and_elements(name);
					
					bool cl_is_initiating =
						c_and_l.loader_is_recorded_as_initiating(class_loader);

					return same_name && cl_is_initiating;
				}
			);

		if(possible_c_and_il.has_no_value()) {
			return {};
		}

		return possible_c_and_il.get().class_;
	}

	template<typename... Args>
	c& emplace_back(Args&&... args) {
		class_and_initiating_loaders& c_and_il = base_type::emplace_back(
			forward<Args>(args)...
		);
		return c_and_il.class_;
	}

	template<basic_range Name>
	expected<c&, reference>
	try_load_non_array_class_by_bootstrap_class_loader(
		Name&& name
	);

	template<basic_range Name>
	c& load_non_array_class_by_bootstrap_class_loader(Name&& name) {
		expected<c&, reference> result
			= try_load_non_array_class_by_bootstrap_class_loader(
				forward<Name>(name)
			);
		if(result.is_unexpected()) {
			print::err(
				"couldn't load non-array class ", name,
				" by bootstrap class-loader\n"
			);
			posix::abort();
		}
		return result.get_expected();
	}

	template<basic_range Name>
	expected<c&, reference>
	try_load_non_array_class_by_user_class_loader(
		Name&& name, object* l_ref
	);

	template<basic_range Name>
	expected<c&, reference> try_load_array_class(
		Name&& name, object* l_ref
	);

	template<basic_range Name>
	c& load_array_class(
		Name&& name, object* l_ref
	) {
		expected<c&, reference> r = try_load_array_class(
			forward<Name>(name), move(l_ref)
		);
		if(r.is_unexpected()) {
			print::err("couldn't load array class ", name, "\n");
			posix::abort();
		}
		return r.get_expected();
	}

private:
	inline expected<c&, reference> try_define_class0(
		posix::memory_for_range_of<uint8> bytes,
		object* defining_loader
	);
public:

	template<basic_range Name>
	expected<c&, reference> try_define_class(
		Name&& name,
		posix::memory_for_range_of<uint8> bytes,
		object* defining_loader // L
	) {
		mutex_->lock();
		on_scope_exit unlock_classes_mutex { [&] {
			mutex_->unlock();
		}};

		/* 1. First, the Java Virtual Machine determines whether L has already
		      been recorded as an initiating loader of a class or interface
		      denoted by N. If so, this derivation attempt is invalid and
		      derivation throws a LinkageError. */
		{
			optional<c&> c
				= try_find_class_which_loading_was_initiated_by(
					name,
					defining_loader
				);
			if(c.has_value()) {
				return unexpected { try_create_linkage_error().get() };
			}
		}

		return try_define_class0(move(bytes), defining_loader);
	}

	template<basic_range Name>
	c& define_array_class(Name&& name, object* defining_loader);

	template<basic_range Name>
	c& define_primitive_class(Name&& name, char ch);

	template<basic_range Name>
	expected<c&, reference>
	try_load_class_by_bootstrap_class_loader(Name&& name) {
		return range{name}.starts_with('[') ?
			try_load_array_class(forward<Name>(name), nullptr)
			:
			try_load_non_array_class_by_bootstrap_class_loader(
				forward<Name>(name)
			);
	}

	template<basic_range Name>
	c& load_class_by_bootstrap_class_loader(Name&& name) {
		expected<c&, reference> result
			= try_load_class_by_bootstrap_class_loader(forward<Name>(name));
		if(result.is_unexpected()) {
			print::err(
				"couldn't load class ", name,
				" by bootstrap class-loader\n"
			);
			posix::abort();
		}
		return result.get_expected();
	}

	template<basic_range Name>
	expected<c&, reference>
	try_load_non_array_class(Name&& name, object* l_ref) {
		return l_ref == nullptr ?
			try_load_non_array_class_by_bootstrap_class_loader(
				forward<Name>(name)
			)
			:
			try_load_non_array_class_by_user_class_loader(
				forward<Name>(name), move(l_ref)
			);
	}

	template<basic_range Name>
	expected<c&, reference>
	try_load_class(Name&& name, object* class_loader) {
		if(range{name}.starts_with('[')) {
			return try_load_array_class(
				forward<Name>(name), class_loader
			);
		}

		return class_loader == nullptr ?
			try_load_non_array_class_by_bootstrap_class_loader(
				forward<Name>(name)
			)
			:
			try_load_non_array_class_by_user_class_loader(
				forward<Name>(name), class_loader
			);
	}

} classes{ posix::allocate_memory_for<class_and_initiating_loaders>(65536) };

/* The process of loading and creating the nonarray class or interface C denoted
by N using the bootstrap class loader is as follows. */
template<basic_range Name>
expected<c&, reference>
classes::try_load_non_array_class_by_bootstrap_class_loader(
	Name&& name
) {
	mutex_->lock();
	on_scope_exit unlock_classes_mutex { [&] {
		mutex_->unlock();
	}};

	/* First, the Java Virtual Machine determines whether the bootstrap class
	   loader has already been recorded as an initiating loader of a class or
	   interface denoted by N. If so, this class or interface is C, and no class
	   loading or creation is necessary. */
	optional<c&> loaded_class
		= try_find_class_which_loading_was_initiated_by(
			name, nullptr
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
		.view_copied_elements_on_stack([&](span<char> root_path_on_stack) {
			return try_load_class_file_data_at(
				root_path_on_stack,
				name
			);
		});

	if(!possible_data.has_value()) {
		expected<reference, reference> possible_name_ref
			= try_create_string_from_utf8(name);
		if(possible_name_ref.is_unexpected()) {
			return possible_name_ref.move_unexpected();
		}
		reference name_ref = possible_name_ref.move_expected();
		return try_create_class_not_found_exception(move(name_ref)).get();
	}

	auto data = possible_data.move();

	return try_define_class(forward<Name>(name), move(data), nullptr);
}

/* The process of loading and creating the nonarray class or interface C denoted
   by N using a user-defined class loader L is as follows. */
template<basic_range Name>
expected<c&, reference>
classes::try_load_non_array_class_by_user_class_loader(
	Name&& name, object* l_ref
) {
	if(l_ref == nullptr) {
		posix::abort();
	}
	c& l_c = l_ref->c();

	mutex_->lock();
	on_scope_exit unlock_classes_mutex { [&] {
		mutex_->unlock();
	}};

	/* First, the Java Virtual Machine determines whether L has already been
	   recorded as an initiating loader of a class or interface denoted by N. If
	   so, this class or interface is C, and no class loading or creation is
	   necessary. */
	optional<c&> loaded_class
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
	expected<reference, reference> possible_name_ref
		= try_create_string_from_utf8(name);
	if(possible_name_ref.is_unexpected()) {
		return unexpected{ possible_name_ref.move_unexpected() };
	}
	reference name_ref = possible_name_ref.move_expected();
	method& load_method = l_c[class_loader_load_class_method_index];

	optional<reference> possible_exception
		= try_execute(load_method, reference{*l_ref}, name_ref);
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
	if(c.defining_loader().object_ptr() != l_ref) {
		mark_class_loader_as_initiating_for_class(c, l_ref);
	}
	return c;
}

/* The following steps are used to create the array class C denoted by the name
   N in association with the class loader L. L may be either the bootstrap class
   loader or a user-defined class loader. */
template<basic_range Name>
expected<c&, reference> classes::try_load_array_class(
	Name&& name, object* l_ref
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
		= try_find_class_which_loading_was_initiated_by(name, l_ref);

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
	char component_type = name[1];

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
					return try_load_class(cn, l_ref);
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

	object* defining_class_loader
		= component_class.defining_loader().object_ptr();

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
	
	if(component_class.defining_loader().object_ptr() != l_ref) {
		mark_class_loader_as_initiating_for_class(c, l_ref);
	}

	/*    If the component type is a reference type, the accessibility of the
	      array class is determined by the accessibility of its component type
	      (§5.4.4). Otherwise, the array class is accessible to all classes and
	      interfaces. */
	// TODO

	return c;
}

template<basic_range Name>
c& classes::define_array_class(
	Name&& name, object* defining_loader
) {
	mutex_->lock();
	on_scope_exit unlock_classes_mutex { [&] {
		mutex_->unlock();
	}};

	auto data = posix::allocate_memory_for<uint8>(name.size());
	span<char> data_as_span{ (char*) data.iterator(), data.size() };
	range{ name }.copy_to(data_as_span);

	posix::memory_for_range_of<instance_field> declared_instance_fields {
		posix::allocate_memory_for<instance_field>(2)
	};

	// ptr to data
	declared_instance_fields[0].construct(
		class_file::access_flags{ class_file::access_flag::_private },
		class_file::constant::utf8{ nullptr, 0 },
		class_file::constant::utf8{ c_string{ "J" } }
	);
	// length
	declared_instance_fields[1].construct(
		class_file::access_flags{ class_file::access_flag::_private },
		class_file::constant::utf8{ nullptr, 0 },
		class_file::constant::utf8{ c_string{ "I" } }
	);

	auto descriptor = posix::allocate_memory_for<uint8>(range_size(name));
	name.copy_to(span{ (char*) descriptor.iterator(), range_size(descriptor) });

	return emplace_back(
		constants{}, bootstrap_methods{},
		move(data),
		class_file::access_flags{ class_file::access_flag::_public },
		this_class_name { data_as_span },
		move(descriptor),
		class_file::constant::utf8{},
		object_class.get(),
		posix::memory_for_range_of<c*>{},
		posix::memory_for_range_of<static_field>{},
		move(declared_instance_fields),
		posix::memory_for_range_of<static_method>{},
		posix::memory_for_range_of<instance_method>{},
		optional<method>{},
		is_array_class{ true },
		is_primitive_class{ false },
		defining_loader == nullptr ? nullptr_ref : reference{ *defining_loader }
	);
}

template<basic_range Name>
c& classes::define_primitive_class(Name&& name, char ch) {
	// should be called at the beginning, no lock needed

	auto data = posix::allocate_memory_for<uint8>(name.size());
	span<char> data_span{ (char*) data.iterator(), data.size() };
	range{ name }.copy_to(data_span);

	auto descriptor = posix::allocate_memory_for<uint8>(1);
	descriptor[0].construct((uint8)ch);

	return emplace_back(
		constants{}, bootstrap_methods{},
		move(data),
		class_file::access_flags{ class_file::access_flag::_public },
		this_class_name{ data_span },
		move(descriptor),
		class_file::constant::utf8{},
		object_class,
		posix::memory_for_range_of<c*>{},
		posix::memory_for_range_of<static_field>{},
		posix::memory_for_range_of<instance_field>{},
		posix::memory_for_range_of<static_method>{},
		posix::memory_for_range_of<instance_method>{},
		optional<method>{},
		is_array_class{ false },
		is_primitive_class{ true }
	);
}
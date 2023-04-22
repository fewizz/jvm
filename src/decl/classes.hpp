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
			c._class.destruct_declared_static_fields_values();
		}
	}

private:
	void mark_class_loader_as_initiating_for_class(_class& c, object* cl) {
		for(auto& c_and_cl : *this) {
			if(&c_and_cl._class == &c) {
				c_and_cl.record_as_initiating(cl);
				return;
			}
		}
		posix::abort();
	}
public:

	template<basic_range Name>
	optional<_class&> try_find_class_which_loading_was_initiated_by(
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
						c_and_l._class.name().has_equal_size_and_elements(name);
					
					bool cl_is_initiating =
						c_and_l.loader_is_recorded_as_initiating(class_loader);

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
	expected<_class&, reference>
	try_load_non_array_class_by_bootstrap_class_loader(
		Name&& name
	);

	template<basic_range Name>
	_class& load_non_array_class_by_bootstrap_class_loader(Name&& name) {
		expected<_class&, reference> result
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
	expected<_class&, reference>
	try_load_non_array_class_by_user_class_loader(
		Name&& name, object* l_ref
	);

	template<basic_range Name>
	expected<_class&, reference> try_load_array_class(
		Name&& name, object* l_ref
	);

	template<basic_range Name>
	_class& load_array_class(
		Name&& name, object* l_ref
	) {
		expected<_class&, reference> r = try_load_array_class(
			forward<Name>(name), move(l_ref)
		);
		if(r.is_unexpected()) {
			print::err("couldn't load array class ", name, "\n");
			posix::abort();
		}
		return r.get_expected();
	}

	template<basic_range Name>
	expected<_class&, reference> try_define_class(
		Name&& n,
		posix::memory_for_range_of<uint8> bytes,
		object* defining_loader // L
	);

	template<basic_range Name>
	_class& define_array_class(Name&& name, object* defining_loader);

	template<basic_range Name>
	_class& define_primitive_class(Name&& name, char ch);

	template<basic_range Name>
	expected<_class&, reference>
	try_load_class_by_bootstrap_class_loader(Name&& name) {
		return range{name}.starts_with('[') ?
			try_load_array_class(forward<Name>(name), nullptr)
			:
			try_load_non_array_class_by_bootstrap_class_loader(
				forward<Name>(name)
			);
	}

	template<basic_range Name>
	_class& load_class_by_bootstrap_class_loader(Name&& name) {
		expected<_class&, reference> result
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
	expected<_class&, reference>
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
	expected<_class&, reference>
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
expected<_class&, reference>
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
	optional<_class&> loaded_class
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
			return move(possible_name_ref.get_unexpected());
		}
		reference name_ref = move(possible_name_ref.get_expected());
		return try_create_class_not_found_exception(move(name_ref)).get();
	}

	auto data = move(possible_data.get());

	return try_define_class(forward<Name>(name), move(data), nullptr);
}

/* The process of loading and creating the nonarray class or interface C denoted
   by N using a user-defined class loader L is as follows. */
template<basic_range Name>
expected<_class&, reference>
classes::try_load_non_array_class_by_user_class_loader(
	Name&& name, object* l_ref
) {
	if(l_ref == nullptr) {
		posix::abort();
	}
	_class& l_c = l_ref->_class();

	mutex_->lock();
	on_scope_exit unlock_classes_mutex { [&] {
		mutex_->unlock();
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
	expected<reference, reference> possible_name_ref
		= try_create_string_from_utf8(name);
	if(possible_name_ref.is_unexpected()) {
		return unexpected{ move(possible_name_ref.get_unexpected()) };
	}
	reference name_ref = move(possible_name_ref.get_expected());
	method& load_method = l_c[class_loader_load_class_method_index];

	optional<reference> possible_exception
		= try_execute(load_method, reference{*l_ref}, name_ref);
	if(possible_exception.has_value()) {
		return move(possible_exception.get());
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
expected<_class&, reference> classes::try_load_array_class(
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
	
	optional<_class&> loaded_class
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

	expected<_class&, reference> possible_component_class = [&]()
	-> expected<_class&, reference> {
		if(component_is_reference) {
			auto iter = name.iterator() + 1;
			nuint size = name.size() - 1;

			if(component_is_class) {
				++iter; // skip L
				--size;

				--size; // skip ';'
			}

			expected<_class&, reference> possible_component_class
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
		return { move(possible_component_class.get_unexpected()) };
	}

	_class& component_class = possible_component_class.get_expected();

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
			_class& c = c_and_il._class;
			return
				c.name().has_equal_size_and_elements(name) &&
				c.defining_loader().object_ptr() == defining_class_loader;
		}
	);

	_class& c = c_and_l.has_value() ?
		c_and_l.get()._class :
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

/* The following steps are used to derive a nonarray class or interface C
   denoted by N from a purported representation in class file format using the
   class loader L. */
template<basic_range Name>
expected<_class&, reference> classes::try_define_class(
	Name&& n,
	posix::memory_for_range_of<uint8> bytes,
	object* defining_loader // L
) {
	mutex_->lock();
	on_scope_exit unlock_classes_mutex { [&] {
		mutex_->unlock();
	}};

	/* 1. First, the Java Virtual Machine determines whether L has already been
	      recorded as an initiating loader of a class or interface denoted by N.
	      If so, this derivation attempt is invalid and derivation throws a
	      LinkageError. */
	{
		optional<_class&> c
			= try_find_class_which_loading_was_initiated_by(
				n,
				defining_loader
			);
		if(c.has_value()) {
			// TODO throw LinkageError
			posix::abort();
		}
	}

	/* 2. Otherwise, the Java Virtual Machine attempts to parse the purported
	      representation. The purported representation may not in fact be a
	      valid representation of C, so derivation must detect the following
	      problems: */

	/*    * If the purported representation is not a ClassFile structure
	        (§4.1, §4.8), derivation throws a ClassFormatError. */

	/*    * Otherwise, if the purported representation is not of a supported
	        major or minor version (§4.1), derivation throws an
	        UnsupportedClassVersionError. */

	/*    * Otherwise, if the purported representation does not actually
	        represent a class or interface named N, derivation throws a
	        NoClassDefFoundError.
	        This occurs when the purported representation has either a
	        this_class item which specifies a name other than N, or an
	        access_flags item which has the ACC_MODULE flag set. */

	class_file::reader magic_reader{ (uint8*) bytes.iterator() };

	auto [magic_exists, version_reader] =
		magic_reader.read_and_check_and_get_version_reader();

	if(!magic_exists) {
		posix::abort();
	}

	auto [version, constant_pool_reader] =
		version_reader.read_and_get_constant_pool_reader();

	uint16 constants_count = constant_pool_reader.read_count();
	::list const_pool_raw {
		posix::allocate_memory_for<::constant>(constants_count)
	};

	auto access_flags_reader {
		constant_pool_reader.read_and_get_access_flags_reader(
			[&]<typename Type>(Type x) {
				if constexpr(same_as<class_file::constant::unknown, Type>) {
					posix::abort();
				}
				else {
					const_pool_raw.emplace_back(x);
				}
			}
		)
	};

	constants const_pool = const_pool_raw.move_storage_range();

	auto [access_flags, this_class_reader] {
		access_flags_reader.read_and_get_this_class_reader()
	};
	auto [this_class_index, super_class_reader] {
		this_class_reader.read_and_get_super_class_reader()
	};
	auto [super_class_index, interfaces_reader] {
		super_class_reader.read_and_get_interfaces_reader()
	};

	/* 3. If C has a direct superclass, the symbolic reference from C to its
	      direct superclass is resolved using the algorithm of §5.4.3.1. Note
	      that if C is an interface it must have Object as its direct
	      superclass, which must already have been loaded. Only Object has no
	      direct superclass. */
	optional<_class&> super;

	if(super_class_index > 0) {
		class_file::constant::_class super_class_constant {
			const_pool.class_constant(super_class_index)
		};
		class_file::constant::utf8 super_class_name {
			const_pool.utf8_constant(super_class_constant.name_index)
		};
		expected<_class&, reference> possible_super_class
			= try_load_non_array_class(
				super_class_name, defining_loader
			);
		if(possible_super_class.is_unexpected()) {
			return { possible_super_class.get_unexpected() };
		}
		// TODO access control
		super = possible_super_class.get_expected();
	}

	/* 4. If C has any direct superinterfaces, the symbolic references from C to
	      its direct superinterfaces are resolved using the algorithm of
	      §5.4.3.1. */
	::list interfaces = posix::allocate_memory_for<_class*>(
		interfaces_reader.read_count()
	);

	reference thrown;

	auto fields_reader =
		interfaces_reader.read_and_get_fields_reader(
			[&](class_file::constant::class_index interface_index)
			{
				if(!thrown.is_null()) {
					return;
				}

				class_file::constant::_class c =
					const_pool.class_constant(interface_index);
				class_file::constant::utf8 interface_name =
					const_pool.utf8_constant(c.name_index);
				expected<_class&, reference> possible_interface
					= try_load_non_array_class(
						interface_name, defining_loader
					);
				if(possible_interface.is_unexpected()) {
					thrown = possible_interface.get_unexpected();
					return;
				}
				// TODO access control
				interfaces.emplace_back(&possible_interface.get_expected());
			}
		);
	
	if(!thrown.is_null()) {
		return { thrown };
	}
	
	uint16 fields_count = fields_reader.read_count();

	::list fields {
		posix::allocate_memory_for<field>(fields_count)
	};

	auto methods_reader = fields_reader.read_and_get_methods_reader(
		[&](auto field_reader) {
			auto [access_flags, name_index_reader] {
				field_reader.read_access_flags_and_get_name_index_reader()
			};
			auto [name_index, descriptor_index_reader] {
				name_index_reader.read_and_get_descriptor_index_reader()
			};
			auto [descriptor_index, attributes_reader] {
				descriptor_index_reader.read_and_get_attributes_reader()
			};
			auto it = attributes_reader.read_and_get_advanced_iterator(
				[&](auto name_index) {
					return const_pool.utf8_constant(name_index);
				},
				[&]<typename Type>(Type) {
				}
			);
			class_file::constant::utf8 name =
				const_pool.utf8_constant(name_index);
			class_file::constant::utf8 descriptor =
				const_pool.utf8_constant(descriptor_index);
			fields.emplace_back(access_flags, name, descriptor);
			return it;
		}
	);

	::list methods {
		posix::allocate_memory_for<method>(methods_reader.read_count())
	};

	auto read_method_and_get_advaned_iterator = []<typename Iterator>(
		constants& const_pool, class_file::method::reader<Iterator> reader
	) -> tuple<method, Iterator> {
		auto [access_flags, name_index_reader] {
			reader.read_access_flags_and_get_name_index_reader()
		};
		auto [name_index, descriptor_index_reader] {
			name_index_reader.read_and_get_descriptor_index_reader()
		};
		auto [desc_index, attributes_reader] {
			descriptor_index_reader.read_and_get_attributes_reader()
		};

		code_or_native_function_ptr code_or_native_function {
			optional<native_function_ptr>()
		};

		posix::memory_for_range_of<
			class_file::attribute::code::exception_handler
		> exception_handlers{};

		posix::memory_for_range_of<
			tuple<uint16, class_file::line_number>
		> line_numbers{};

		auto mapper = [&](auto name_index) {
			return const_pool.utf8_constant(name_index);
		};

		Iterator it = attributes_reader.read_and_get_advanced_iterator(
			mapper, [&]<typename Type>(Type reader) {

			using namespace class_file;

			if constexpr (Type::attribute_type == attribute::type::code) {
				using namespace attribute::code;
				Type max_stack_reader = reader;

				auto [max_stack, max_locals_reader]
					= max_stack_reader.read_and_get_max_locals_reader();
				auto [max_locals, code_reader]
					= max_locals_reader.read_and_get_code_reader();

				auto code_span = code_reader.read_as_span();
				auto exception_table_reader
					= code_reader.skip_and_get_exception_table_reader();

				code_or_native_function = ::code {
					code_span, max_stack, max_locals
				};

				::list exception_handlers_list {
					posix::allocate_memory_for<
						class_file::attribute::code::exception_handler
					>(exception_table_reader.read_count())
				};

				auto attributes_reader
					= exception_table_reader.read_and_get_attributes_reader(
					[&](exception_handler eh) {
						exception_handlers_list.emplace_back(eh);
						return loop_action::next;
					}
				);

				attributes_reader.read_and_get_advanced_iterator(
					mapper,
					[&]<typename CodeAttributeType>(
						CodeAttributeType reader
					) {
						using namespace class_file;

						if constexpr (
							CodeAttributeType::attribute_type ==
							attribute::type::line_numbers
						) {
							CodeAttributeType count_reader = reader;
							auto [count, line_numbers_reader]
								= count_reader
								.read_and_get_line_numbers_reader();
							
							::list line_numbers_list {
								posix::allocate_memory_for<
									tuple<uint16, class_file::line_number>
								>(count)
							};

							line_numbers_reader.read_and_get_advanced_iterator(
								count,
								[&](
									uint16 start_pc,
									class_file::line_number ln
								) {
									line_numbers_list.emplace_back(
										start_pc,
										ln
									);
								}
							);
							line_numbers
								= line_numbers_list.move_storage_range();
						}
					}
				);

				exception_handlers
					= exception_handlers_list.move_storage_range();
			}
		});

		class_file::constant::utf8 name = const_pool.utf8_constant(name_index);
		class_file::constant::utf8 desc = const_pool.utf8_constant(desc_index);

		return {
			method {
				access_flags,
				name,
				desc,
				code_or_native_function,
				move(exception_handlers),
				move(line_numbers)
			},
			it
		};
	};

	auto attributes_reader = methods_reader.read_and_get_attributes_reader(
		[&](auto method_reader) {
			auto [m, it] = read_method_and_get_advaned_iterator(
				const_pool, method_reader
			);
			methods.emplace_back(move(m));
			return it;
		}
	);

	bootstrap_methods bootstrap_methods{};
	class_file::constant::utf8 source_file{};

	auto read_bootstap_methods = [](auto reader) -> ::bootstrap_methods {
		auto [count, bootstrap_methods_reader] {
			reader.read_count_and_get_methods_reader()
		};

		::list bootstrap_methods_raw {
			posix::allocate_memory_for<bootstrap_method>(count)
		};

		bootstrap_methods_reader.read(
			count,
			[&](auto method_reader) {
				auto [reference_index, arguments_count_reader] {
					method_reader
						.read_reference_index_and_get_arguments_count_reader()
				};
				auto [arguments_count, arguments_reader] {
					arguments_count_reader.read_and_get_arguments_reader()
				};

				::list arguments_indices_raw {
					posix::allocate_memory_for<
						class_file::constant::index
					>(arguments_count)
				};

				arguments_reader.read(
					arguments_count,
					[&](class_file::constant::index index) {
						arguments_indices_raw.emplace_back(index);
					}
				);

				bootstrap_methods_raw.emplace_back(
					reference_index, arguments_indices_raw.move_storage_range()
				);
			}
		);

		return ::bootstrap_methods {
			bootstrap_methods_raw.move_storage_range()
		};
	};

	attributes_reader.read_and_get_advanced_iterator(
		[&](auto attribute_name_index) {
			return const_pool.utf8_constant(attribute_name_index);
		},
		[&]<typename Type>(Type attribute_reader) {
			if constexpr(
				Type::attribute_type ==
				class_file::attribute::type::bootstrap_methods
			) {
				bootstrap_methods = read_bootstap_methods(attribute_reader);
			}
			if constexpr(
				Type::attribute_type == class_file::attribute::type::source_file
			) {
				Type index_reader = attribute_reader;
				auto [utf8_index, it]
					= index_reader.read_and_get_advanced_iterator();
				
				source_file = const_pool.utf8_constant(utf8_index);
			}
		}
	);

	class_file::constant::_class this_class_constant {
		const_pool.class_constant(this_class_index)
	};
	class_file::constant::utf8 name {
		const_pool.utf8_constant(this_class_constant.name_index)
	};

	auto descriptor = posix::allocate_memory_for<uint8>(name.size() + 2);
	name.copy_to(
		span{ (char*) descriptor.iterator() + 1, descriptor.size() - 2 }
	);
	descriptor[0].construct((uint8)'L');
	descriptor[descriptor.size() - 1].construct((uint8)';');

	/* If no exception is thrown in steps 1-4, then derivation of the class or
	   interface C succeeds. The Java Virtual Machine marks C to have L as its
	   defining loader, records that L is an initiating loader of C (§5.3.4),
	   and creates C in the method area (§2.5.4). */
	return emplace_back(
		move(const_pool), move(bootstrap_methods),
		move(bytes), access_flags,
		this_class_name{ name },
		move(descriptor),
		source_file,
		super,
		interfaces.move_storage_range(),
		fields.move_storage_range(),
		methods.move_storage_range(),
		is_array_class{ false },
		is_primitive_class{ false },
		defining_loader == nullptr ? nullptr_ref : reference{ *defining_loader }
	);
}

template<basic_range Name>
_class& classes::define_array_class(
	Name&& name, object* defining_loader
) {
	mutex_->lock();
	on_scope_exit unlock_classes_mutex { [&] {
		mutex_->unlock();
	}};

	auto data = posix::allocate_memory_for<uint8>(name.size());
	span<char> data_as_span{ (char*) data.iterator(), data.size() };
	range{ name }.copy_to(data_as_span);

	posix::memory_for_range_of<field> declared_fields {
		posix::allocate_memory_for<field>(2)
	};

	// ptr to data
	declared_fields[0].construct(
		class_file::access_flags{ class_file::access_flag::_private },
		class_file::constant::utf8{ nullptr, 0 },
		class_file::constant::utf8{ c_string{ "J" } }
	);
	// length
	declared_fields[1].construct(
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
		posix::memory_for_range_of<_class*>{},
		move(declared_fields),
		posix::memory_for_range_of<method>{},
		is_array_class{ true },
		is_primitive_class{ false },
		defining_loader == nullptr ? nullptr_ref : reference{ *defining_loader }
	);
}

template<basic_range Name>
_class& classes::define_primitive_class(Name&& name, char ch) {
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
		posix::memory_for_range_of<_class*>{},
		posix::memory_for_range_of<field>{},
		posix::memory_for_range_of<method>{},
		is_array_class{ false },
		is_primitive_class{ true }
	);
}
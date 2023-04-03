#include "define/class.hpp"

#include "./read_method.hpp"
#include "./read_bootstrap_methods.hpp"
#include "classes.hpp"

#include <integer.hpp>
#include <range.hpp>

#include <class_file/reader.hpp>

/* The following steps are used to derive a nonarray class or interface C
   denoted by N from a purported representation in class file format using the
   class loader L. */
template<basic_range Name>
static inline _class& define_class(
	Name&& n,
	posix::memory_for_range_of<uint8> bytes,
	reference defining_loader // L
) {
	/* 1. First, the Java Virtual Machine determines whether L has already been
	      recorded as an initiating loader of a class or interface denoted by N.
	      If so, this derivation attempt is invalid and derivation throws a
	      LinkageError. */
	{
		auto& m = classes.mutex();
		m->lock();
		on_scope_exit unlock_classes_mutex { [&] {
			m->unlock();
		}};

		optional<_class&> c
			= classes.try_find_class_which_loading_was_initiated_by(
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
	list const_pool_raw { 
		posix::allocate_memory_for<constant>(constants_count)
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
		_class& super_class = classes.load_non_array_class(
			super_class_name, defining_loader
		);
		// TODO access control
		super = super_class;
	}

	/* 4. If C has any direct superinterfaces, the symbolic references from C to
	      its direct superinterfaces are resolved using the algorithm of
	      §5.4.3.1. */
	list interfaces = posix::allocate_memory_for<_class*>(
		interfaces_reader.read_count()
	);

	auto fields_reader =
		interfaces_reader.read_and_get_fields_reader(
			[&](class_file::constant::interface_index interface_index) {
				class_file::constant::_class c =
					const_pool.class_constant(interface_index);
				class_file::constant::utf8 interface_name =
					const_pool.utf8_constant(c.name_index);
				_class& interface = classes.load_non_array_class(
					interface_name, defining_loader
				);
				// TODO access control
				interfaces.emplace_back(&interface);
			}
		);
	
	uint16 fields_count = fields_reader.read_count();

	list<posix::memory_for_range_of<field>> fields {
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

	list<posix::memory_for_range_of<method>> methods {
		posix::allocate_memory_for<method>(methods_reader.read_count())
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

	{
		auto& m = classes.mutex();
		m->lock();
		on_scope_exit unlock_classes_mutex { [&] {
			m->unlock();
		}};

		return classes.emplace_back(
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
			move(defining_loader)
		);
	}
}
#pragma once

#include "decl/classes.hpp"

#include "./define_class.inc/read_method.hpp"
#include "./define_class.inc/read_field.hpp"
#include "./define_class.inc/read_bootstrap_methods.hpp"

/* The following steps are used to derive a nonarray class or interface C
   denoted by N from a purported representation in class file format using the
   class loader L. */
template<basic_range Name>
expected<c&, reference> classes::try_define_class(
	Name&& name,
	posix::memory<> bytes,
	j::c_loader* defining_loader // L
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

	class_file::reader class_reader{ (uint8*) bytes.iterator() };

	auto [magic_exists, version_reader] =
		class_reader.read_and_check_magic_and_get_version_reader();

	if(!magic_exists) {
		posix::abort();
	}

	auto [version, constant_pool_reader] =
		version_reader.read_and_get_constant_pool_reader();

	uint16 constants_count = constant_pool_reader.get_count();
	::list const_pool_raw {
		posix::allocate<::constant>(constants_count)
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
	auto [this_class_constant_index, super_class_reader] {
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
	optional<c&> super;

	if(super_class_index > 0) {
		class_file::constant::_class super_class_constant {
			const_pool[super_class_index]
		};
		class_file::constant::utf8 super_class_name {
			const_pool[super_class_constant.name_index]
		};
		expected<c&, reference> possible_super_class
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
	::list interfaces = posix::allocate<c*>(
		interfaces_reader.get_count()
	);

	reference thrown;

	auto fields_reader = interfaces_reader.read_and_get_fields_reader(
		[&](class_file::constant::class_index interface_index) {
			if(!thrown.is_null()) {
				return;
			}

			class_file::constant::_class c = const_pool[interface_index];
			class_file::constant::utf8 interface_name
				= const_pool[c.name_index];
			expected<::c&, reference> possible_interface
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

	::list fields {
		posix::allocate<field>(fields_reader.get_count())
	};

	nuint static_fields_count = 0;
	nuint instance_fields_count = 0;

	auto methods_reader = fields_reader.read_and_get_methods_reader(
		[&](auto field_reader) {
			auto [f, it] = read_field(const_pool, field_reader);

			++(f.is_static() ? static_fields_count : instance_fields_count);

			fields.emplace_back(move(f));

			return it;
		}
	);

	::list static_fields {
		posix::allocate<static_field>(static_fields_count)
	};

	::list instance_fields {
		posix::allocate<instance_field>(instance_fields_count)
	};

	for(field& f : fields) {
		if(f.is_static()) {
			static_fields.emplace_back(move(f));
		}
		else {
			instance_fields.emplace_back(move(f));
		}
	}

	::list methods {
		posix::allocate<method>(methods_reader.get_count())
	};

	nuint static_methods_count = 0;
	nuint instance_methods_count = 0;

	auto attributes_reader = methods_reader.read_and_get_attributes_reader(
		[&](auto method_reader) {
			auto [m, it] = read_method(const_pool, method_reader);
			if(!m.is_class_initialisation()) {
				++(
					m.is_static() ?
					static_methods_count :
					instance_methods_count
				);
			}
			methods.emplace_back(move(m));
			return it;
		}
	);

	::list static_methods {
		posix::allocate<static_method>(static_methods_count)
	};
	::list instance_methods {
		posix::allocate<instance_method>(instance_methods_count)
	};
	optional<method> initialisation_method{};

	for(method& m : methods) {
		if(m.is_class_initialisation()) {
			initialisation_method = move(m);
			continue;
		}

		if(m.is_static()) {
			static_methods.emplace_back(move(m));
		}
		else {
			instance_methods.emplace_back(move(m));
		}
	}

	bootstrap_methods bootstrap_methods{};
	class_file::constant::utf8 source_file{};

	attributes_reader.read(
		[&](auto attribute_name_index) {
			return const_pool[attribute_name_index];
		},
		[&]<typename Type>(Type reader) {
			if constexpr(
				Type::attribute_type ==
				class_file::attribute::type::bootstrap_methods
			) {
				bootstrap_methods = read_bootstap_methods(reader);
			}
			if constexpr(
				Type::attribute_type == class_file::attribute::type::source_file
			) {
				auto [utf8_constant_index, it]
					= reader.read_index_and_get_advanced_iterator();
				source_file = const_pool[utf8_constant_index];
			}
		}
	);

	class_file::constant::_class this_class_constant {
		const_pool[this_class_constant_index]
	};
	class_file::constant::utf8 name_utf8 {
		const_pool[this_class_constant.name_index]
	};

	class_data data{};
	data.emplace_back(move(bytes));
	data.emplace_back(posix::allocate(name_utf8.size() + 2));
	name_utf8.copy_to(
		span {
			data[1].as_span().iterator() + 1,
			name_utf8.size()
		}
	);
	data[1][0] = u8'L';
	data[1][data[1].size() - 1] = u8';';

	class_file::constant::utf8 descriptor
		= data[1].cast<utf8::unit, uint16>();

	/* If no exception is thrown in steps 1-4, then derivation of the class or
	   interface C succeeds. The Java Virtual Machine marks C to have L as its
	   defining loader, records that L is an initiating loader of C (§5.3.4),
	   and creates C in the method area (§2.5.4). */
	return emplace_back(
		move(const_pool),
		move(bootstrap_methods),
		move(data),
		access_flags,
		name_utf8,
		descriptor,
		source_file,
		super,
		interfaces.move_storage_range(),
		static_fields.move_storage_range(),
		instance_fields.move_storage_range(),
		static_methods.move_storage_range(),
		instance_methods.move_storage_range(),
		move(initialisation_method),
		is_array_class{ false },
		is_primitive_class{ false },
		defining_loader == nullptr ? nullptr_ref : reference{ *defining_loader }
	);
}
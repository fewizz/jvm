#include "define/class.hpp"

#include "./read_method.hpp"
#include "./read_bootstrap_methods.hpp"
#include "classes.hpp"

#include <integer.hpp>
#include <range.hpp>

#include <class_file/reader.hpp>

static inline _class& define_class(memory_span bytes) {
	class_file::reader reader{ bytes.elements_ptr() };

	auto [magic_exists, version_reader] =
		reader.check_for_magic_and_get_version_reader();

	if(!magic_exists) {
		fprintf(stderr, "magic doesn't exist");
		abort();
	}

	auto [version, constant_pool_reader] =
		version_reader.read_and_get_constant_pool_reader();

	uint16 constants_count = constant_pool_reader.entries_count();
	constants const_pool { allocate_for<constant>(constants_count) };

	auto access_flags_reader {
		constant_pool_reader.read_and_get_access_flags_reader(
			[&]<typename Type>(Type x) {
				if constexpr(same_as<class_file::constant::unknown, Type>) {
					fprintf(stderr, "unknown constant with tag %hhu", x.tag);
					abort();
				}
				else {
					const_pool.emplace_back(x);
				}
			}
		)
	};

	auto [access_flags, this_class_reader] {
		access_flags_reader.read_and_get_this_class_reader()
	};
	auto [this_class_index, super_class_reader] {
		this_class_reader.read_and_get_super_class_reader()
	};
	auto [super_class_index, interfaces_reader] {
		super_class_reader.read_and_get_interfaces_reader()
	};

	declared_interfaces interfaces {
		allocate_for<_class&>(interfaces_reader.count())
	};

	auto fields_reader =
		interfaces_reader.read_and_get_fields_reader(
			[&](class_file::constant::interface_index interface_index) {
				class_file::constant::_class c =
					const_pool.class_constant(interface_index);
				class_file::constant::utf8 name =
					const_pool.utf8_constant(c.name_index);
				_class& interface = classes.find_or_load(name);
				interfaces.emplace_back(interface);
			}
		);

	uint16 fields_count = fields_reader.count();

	declared_fields fields { allocate_for<field>(fields_count) };

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

	declared_methods methods{ allocate_for<method>(methods_reader.count()) };

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
		}
	);

	class_file::constant::_class this_class_constant {
		const_pool.class_constant(this_class_index)
	};
	class_file::constant::utf8 name {
		const_pool.utf8_constant(this_class_constant.name_index)
	};

	optional<_class&> super;

	if(super_class_index > 0) {
		class_file::constant::_class super_class_constant {
			const_pool.class_constant(super_class_index)
		};
		class_file::constant::utf8 super_class_name {
			const_pool.utf8_constant(super_class_constant.name_index)
		};
		_class& super_class = classes.find_or_load(super_class_name);
		super = super_class;
	}

	return classes.emplace_back(
		move(const_pool), move(bootstrap_methods),
		bytes, access_flags,
		this_class_name{ name },
		super,
		move(interfaces),
		move(fields),
		move(methods),
		is_array_class{ false },
		is_primitive_class{ false }
	);
}
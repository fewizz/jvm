#include "decl/classes.hpp"

/* The following steps are used to derive a nonarray class or interface C
   denoted by N from a purported representation in class file format using the
   class loader L. */
expected<c&, reference> classes::try_define_class0(
	posix::memory_for_range_of<uint8> bytes,
	object* defining_loader // L
) {
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
	optional<c&> super;

	if(super_class_index > 0) {
		class_file::constant::_class super_class_constant {
			const_pool.class_constant(super_class_index)
		};
		class_file::constant::utf8 super_class_name {
			const_pool.utf8_constant(super_class_constant.name_index)
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
	::list interfaces = posix::allocate_memory_for<c*>(
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
		posix::allocate_memory_for<field>(fields_reader.read_count())
	};

	nuint static_fields_count = 0;
	nuint instance_fields_count = 0;

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

			field f{ access_flags, name, descriptor };

			++(f.is_static() ? static_fields_count : instance_fields_count);

			fields.emplace_back(move(f));

			return it;
		}
	);

	::list static_fields {
		posix::allocate_memory_for<static_field>(static_fields_count)
	};

	::list instance_fields {
		posix::allocate_memory_for<instance_field>(instance_fields_count)
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
		posix::allocate_memory_for<method>(methods_reader.read_count())
	};

	nuint static_methods_count = 0;
	nuint instance_methods_count = 0;

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
			native_function_ptr{nullptr}
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
		posix::allocate_memory_for<static_method>(static_methods_count)
	};
	::list instance_methods {
		posix::allocate_memory_for<instance_method>(instance_methods_count)
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
	class_file::constant::utf8 name_utf8 {
		const_pool.utf8_constant(this_class_constant.name_index)
	};

	auto descriptor_utf8 = posix::allocate_memory_for<uint8>(
		name_utf8.size() + 2
	);

	name_utf8.copy_to(
		span {
			(char*) descriptor_utf8.iterator() + 1,
			descriptor_utf8.size() - 2
		}
	);

	descriptor_utf8[0].construct((uint8)'L');
	descriptor_utf8[descriptor_utf8.size() - 1].construct((uint8)';');

	/* If no exception is thrown in steps 1-4, then derivation of the class or
	   interface C succeeds. The Java Virtual Machine marks C to have L as its
	   defining loader, records that L is an initiating loader of C (§5.3.4),
	   and creates C in the method area (§2.5.4). */
	return emplace_back(
		move(const_pool), move(bootstrap_methods),
		move(bytes), access_flags,
		this_class_name{ name_utf8 },
		move(descriptor_utf8),
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
#pragma once

#include <core/range/of_value_type_same_as.hpp>
#include <core/range/equal.hpp>
#include <core/expected.hpp>
#include <core/array.hpp>
#include <core/meta/elements/one_of.hpp>
#include <core/span.hpp>
#include <core/flag_enum.hpp>

enum class class_reader_stage {
	magic,
	version,
	constant_pool,
	access_flags,
	this_class,
	super_class,
	interfaces,
	fields,
	methods,
	attributes
};

struct utf8_info : span<uint8> {
	using span<uint8>::span;
};

struct integer_info {
	int32 value;
};

struct class_info {
	uint16 name_index;
};

struct fieldref_info {
	uint16 class_index;
	uint16 name_and_type_index;
};

struct methodref_info {
	uint16 class_index;
	uint16 name_and_type_index;
};

struct method_type_info {
	uint16 descriptor_index;
};

struct class_file_version {
	uint16 minor;
	uint16 major;
};

struct name_and_type_info {
	uint16 name_index;
	uint16 descriptor_index;
};

enum class access_flag {
	_public    = 0x0001,
	_final     = 0x0010,
	super      = 0x0020,
	interface  = 0x0200,
	abstract   = 0x0400,
	synthetic  = 0x1000,
	annotation = 0x2000,
	_enum      = 0x4000,
	_module    = 0x8000
};

using access_flags = flag_enum<access_flag>;

struct attribute_info {
	uint16 name_index;
	span<uint8> data;
};

struct code_attribute {
	uint16 max_stack;
	uint16 max_locals;
	span<uint8> code;
	uint16 exception_table_length;
};

template<typename Iterator>
struct attributes_reader {
	Iterator& src;

	template<typename Handler>
	void read_attributes(Handler&& handler) {
		array x{ *src++, *src++ };
		uint16 count = (x[0] << 8) | x[1];

		for(uint16 i = 0; i < count; ++i) {
			array x0{ *src++, *src++ };
			uint16 name_index  = (x0[0] << 8) | x0[1];

			array x1{ *src++, *src++, *src++, *src++ };
			uint32 length = 
				(x1[0] << 24) |
				(x1[1] << 16) |
				(x1[2] <<  8) |
				 x1[3];

			handler(attribute_info{ name_index, span{ src, length }});
			src += length;
		}
	}
};

struct field_info {
	access_flags access_flags;
	uint16 name_index;
	uint16 descriptor_index;
};

struct method_info {
	access_flags access_flags;
	uint16 name_index;
	uint16 descriptor_index;
};

struct method_handle_info {
	uint8 reference_kind;
	uint16 reference_index;
};

struct invoke_dynamic_info {
	uint16 bootstrap_method_attr_index;
	uint16 name_and_type_index;
};

struct string_info {
	uint16 string_index;
};

template<
	typename Iterator,
	class_reader_stage Stage = class_reader_stage::magic
>
struct class_reader {

	Iterator src;

	expected<
		class_reader<Iterator, class_reader_stage::version>,
		bool
	>
	read_and_check_magic() const
	requires (Stage == class_reader_stage::magic) {
		auto cpy = src;

		array cafebabe{ 0xCA, 0xFE, 0xBA, 0xBE };
		array x{ *cpy++, *cpy++, *cpy++, *cpy++ };

		if(!range::equal(x, cafebabe)) {
			return false;
		}

		return class_reader<
			Iterator,
			class_reader_stage::version
		>{ cpy };
	}

	elements::of<
		class_reader<Iterator, class_reader_stage::constant_pool>,
		class_file_version
	>
	read_version() const
	requires (Stage == class_reader_stage::version) {
		auto cpy = src;

		array x{ *cpy++, *cpy++, *cpy++, *cpy++ };

		class_file_version version {
			.minor = uint16((x[0] << 8) | x[1]),
			.major = uint16((x[2] << 8) | x[3]),
		};

		return {
			{ cpy },
			version
		};
	}

	template<typename Handler>
	class_reader<Iterator, class_reader_stage::access_flags>
	read_constant_pool(Handler&& handler) const
	requires (Stage == class_reader_stage::constant_pool) {
		auto cpy = src;
		array size0{ *cpy++, *cpy++ };
		uint16 size = (size0[0] << 8) | size0[1];

		for(uint16 i = 1; i < size; ++i) {
			uint8 tag = *cpy++;

			switch (tag) {
				case 1: {
					array x{ *cpy++, *cpy++ };
					uint16 len = (x[0] << 8) | x[1];
					handler(utf8_info{ cpy, len });
					cpy += len;
					break;
				}
				case 3: {
					array x{ *cpy++, *cpy++, *cpy++, *cpy++ };
					int32 value =
						(x[0] << 24) |
						(x[1] << 16) |
						(x[2] <<  8) |
						 x[3];
					handler(integer_info{ value });
					break;
				}
				case 7: {
					array x{ *cpy++, *cpy++ };
					uint16 name_index = (x[0] << 8) | x[1];
					handler(class_info{ name_index });
					break;
				}
				case 8: {
					array x{ *cpy++, *cpy++ };
					uint16 index = (x[0] << 8) | x[1];
					handler(string_info{ index });
					break;
				}
				case 9: {
					array x0{ *cpy++, *cpy++ };
					uint16 class_index = (x0[0] << 8) | x0[1];

					array x1{ *cpy++, *cpy++ };
					uint16 nat_index = (x1[0] << 8) | x1[1];

					handler(fieldref_info{ class_index, nat_index });
					break;
				}
				case 10: {
					array x0{ *cpy++, *cpy++ };
					uint16 class_index = (x0[0] << 8) | x0[1];

					array x1{ *cpy++, *cpy++ };
					uint16 nat_index = (x1[0] << 8) | x1[1];

					handler(methodref_info{ class_index, nat_index });
					break;
				}
				case 12: {
					array x0{ *cpy++, *cpy++ };
					uint16 name_index = (x0[0] << 8) | x0[1];

					array x1{ *cpy++, *cpy++ };
					uint16 desc_index = (x1[0] << 8) | x1[1];

					handler(name_and_type_info{ name_index, desc_index });
					break;
				}
				case 15: {
					uint8 kind = *cpy++;

					array x{ *cpy++, *cpy++ };
					uint16 index = (x[0] << 8) | x[1];

					handler(method_handle_info{ kind, index });
					break;
				}
				case 16: {
					array x{ *cpy++, *cpy++ };
					uint16 desc_index = (x[0] << 8) | x[1];

					handler(method_type_info{ desc_index });
					break;
				}
				case 18: {
					array x0{ *cpy++, *cpy++ };
					uint16 method_attr_index = (x0[0] << 8) | x0[1];

					array x1{ *cpy++, *cpy++ };
					uint16 name_and_type_index = (x1[0] << 8) | x1[1];
					
					handler(invoke_dynamic_info {
						method_attr_index,
						name_and_type_index
					});
					break;
				}
				default: handler(tag);
			}

		}

		return { cpy };
	}

	elements::of<
		class_reader<Iterator, class_reader_stage::this_class>,
		access_flags
	>
	read_access_flags() const
	requires (Stage == class_reader_stage::access_flags) {
		auto cpy = src;

		array x{ *cpy++, *cpy++ };
		return { { cpy }, { (access_flag) ((x[0] << 8) | x[1]) } };
	}

	elements::of<
		class_reader<Iterator, class_reader_stage::super_class>,
		uint16
	>
	read_this_class() const
	requires (Stage == class_reader_stage::this_class) {
		auto cpy = src;
		array x{ *cpy++, *cpy++ };
		return { { cpy }, uint16((x[0] << 8) | x[1]) };
	}

	elements::of<
		class_reader<Iterator, class_reader_stage::interfaces>,
		uint16
	>
	read_super_class() const
	requires (Stage == class_reader_stage::super_class) {
		auto cpy = src;
		array x{ *cpy++, *cpy++ };
		return { { cpy }, uint16((x[0] << 8) | x[1]) };
	}

	template<typename Handler>
	class_reader<Iterator, class_reader_stage::fields>
	read_interfaces(Handler&& handler) const
	requires (Stage == class_reader_stage::interfaces) {
		auto cpy = src;
		array x{ *cpy++, *cpy++ };
		uint16 count = (x[0] << 8) | x[1];

		for(uint16 i = 0; i < count; ++i) {
			array x0{ *cpy++, *cpy++ };
			uint16 index = (x0[0] << 8) | x0[1];
			handler(index);
		}

		return { cpy };
	}

	template<typename Handler>
	class_reader<Iterator, class_reader_stage::methods>
	read_fields(Handler&& handler) const
	requires (Stage == class_reader_stage::fields) {
		auto cpy = src;
		array x{ *cpy++, *cpy++ };
		uint16 count = (x[0] << 8) | x[1];

		for(uint16 i = 0; i < count; ++i) {
			array x0{ *cpy++, *cpy++ };
			access_flags access_flags = (access_flag) ((x0[0] << 8) | x0[1]);

			array x1{ *cpy++, *cpy++ };
			uint16 name_index = (x1[0] << 8) | x1[1];

			array x2{ *cpy++, *cpy++ };
			uint16 descriptor_index = (x2[0] << 8) | x2[1];

			handler(
				field_info{ access_flags, name_index, descriptor_index },
				attributes_reader<Iterator>{ cpy }
			);
		}

		return { cpy };
	}

	template<typename Handler>
	class_reader<Iterator, class_reader_stage::attributes>
	read_methods(Handler&& handler) const
	requires (Stage == class_reader_stage::methods) {
		auto cpy = src;
		array x{ *cpy++, *cpy++ };
		uint16 count = (x[0] << 8) | x[1];

		for(uint16 i = 0; i < count; ++i) {
			array x0{ *cpy++, *cpy++ };
			access_flags access_flags = (access_flag) ((x0[0] << 8) | x0[1]);

			array x1{ *cpy++, *cpy++ };
			uint16 name_index = (x1[0] << 8) | x1[1];

			array x2{ *cpy++, *cpy++ };
			uint16 descriptor_index = (x2[0] << 8) | x2[1];

			handler(
				method_info{ access_flags, name_index, descriptor_index },
				attributes_reader<Iterator>{ cpy }
			);
		}

		return { cpy };
	}

};

template<
	typename Iterator,
	class_reader_stage Stage = class_reader_stage::magic
>
class_reader(Iterator) -> class_reader<Iterator, Stage>;
#pragma once

#include <core/range/of_value_type_same_as.hpp>
#include <core/range/equals.hpp>
#include <core/read.hpp>
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
		uint16 count = read<uint16>(src);

		for(uint16 i = 0; i < count; ++i) {
			uint16 name_index  = read<uint16>(src);
			uint32 length = read<uint32>(src);

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

	elements::of<class_reader<Iterator, class_reader_stage::version>, bool>
	operator () () const
	requires (Stage == class_reader_stage::magic) {
		auto cpy = src;

		array cafebabe{ 0xCA, 0xFE, 0xBA, 0xBE };
		array x{ *cpy++, *cpy++, *cpy++, *cpy++ };

		bool result = range::equals(x, cafebabe);

		return {
			{ cpy },
			result
		};
	}

	elements::of<
		class_reader<Iterator, class_reader_stage::constant_pool>,
		class_file_version
	>
	operator() () const
	requires (Stage == class_reader_stage::version) {
		auto cpy = src;

		auto min = read<uint16>(cpy);
		auto maj = read<uint16>(cpy);

		return {
			{ cpy },
			class_file_version{ min, maj }
		};
	}

	template<typename Handler>
	class_reader<Iterator, class_reader_stage::access_flags>
	operator() (Handler&& handler) const
	requires (Stage == class_reader_stage::constant_pool) {
		auto cpy = src;

		uint16 size = read<uint16>(cpy);

		for(uint16 i = 1; i < size; ++i) {
			uint8 tag = *cpy++;

			switch (tag) {
				case 1: {
					uint16 len = read<uint16>(cpy);
					handler(utf8_info{ cpy, len });
					cpy += len;
					break;
				}
				case 3: {
					int32 value = read<int32>(cpy);
					handler(integer_info{ value });
					break;
				}
				case 7: {
					uint16 name_index = read<uint16>(cpy);
					handler(class_info{ name_index });
					break;
				}
				case 8: {
					uint16 index = read<uint16>(cpy);
					handler(string_info{ index });
					break;
				}
				case 9: {
					uint16 class_index = read<uint16>(cpy);
					uint16 nat_index = read<uint16>(cpy);
					handler(fieldref_info{ class_index, nat_index });
					break;
				}
				case 10: {
					uint16 class_index = read<uint16>(cpy);
					uint16 nat_index = read<uint16>(cpy);
					handler(methodref_info{ class_index, nat_index });
					break;
				}
				case 12: {
					uint16 name_index = read<uint16>(cpy);
					uint16 desc_index = read<uint16>(cpy);
					handler(name_and_type_info{ name_index, desc_index });
					break;
				}
				case 15: {
					uint8 kind = *cpy++;
					uint16 index = read<uint16>(cpy);
					handler(method_handle_info{ kind, index });
					break;
				}
				case 16: {
					uint16 desc_index = read<uint16>(cpy);
					handler(method_type_info{ desc_index });
					break;
				}
				case 18: {
					uint16 method_attr_index = read<uint16>(cpy);
					uint16 name_and_type_index = read<uint16>(cpy);
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
	operator () () const
	requires (Stage == class_reader_stage::access_flags) {
		auto cpy = src;
		auto flags = (access_flag) read<uint16>(cpy);

		return { { cpy }, { flags } };
	}

	elements::of<
		class_reader<Iterator, class_reader_stage::super_class>,
		uint16
	>
	operator () () const
	requires (Stage == class_reader_stage::this_class) {
		auto cpy = src;
		auto ind = read<uint16>(cpy);
		return { { cpy }, ind };
	}

	elements::of<
		class_reader<Iterator, class_reader_stage::interfaces>,
		uint16
	>
	operator () () const
	requires (Stage == class_reader_stage::super_class) {
		auto cpy = src;
		auto ind = read<uint16>(cpy);
		return { { cpy }, ind };
	}

	template<typename Handler>
	class_reader<Iterator, class_reader_stage::fields>
	operator () (Handler&& handler) const
	requires (Stage == class_reader_stage::interfaces) {
		auto cpy = src;

		uint16 count = read<uint16>(cpy);

		for(uint16 i = 0; i < count; ++i) {
			uint16 index = read<uint16>(cpy);
			handler(index);
		}

		return { cpy };
	}

	template<typename Handler>
	class_reader<Iterator, class_reader_stage::methods>
	operator () (Handler&& handler) const
	requires (Stage == class_reader_stage::fields) {
		auto cpy = src;
		uint16 count = read<uint16>(cpy);

		for(uint16 i = 0; i < count; ++i) {
			access_flags access_flags = (access_flag) read<uint16>(cpy);

			uint16 name_index = read<uint16>(cpy);
			uint16 descriptor_index = read<uint16>(cpy);;

			handler(
				field_info{ access_flags, name_index, descriptor_index },
				attributes_reader<Iterator>{ cpy }
			);
		}

		return { cpy };
	}

	template<typename Handler>
	class_reader<Iterator, class_reader_stage::attributes>
	operator () (Handler&& handler) const
	requires (Stage == class_reader_stage::methods) {
		auto cpy = src;
		uint16 count = read<uint16>(cpy);

		for(uint16 i = 0; i < count; ++i) {
			access_flags access_flags = (access_flag) read<uint16>(cpy);

			uint16 name_index = read<uint16>(cpy);
			uint16 descriptor_index = read<uint16>(cpy);

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

enum class code_attribute_reader_stage {
	max_locals,
	max_stack,
	code,
	exception_table,
	attributes
};

template<
	typename Iterator,
	code_attribute_reader_stage Stage = code_attribute_reader_stage::max_locals
>
struct code_attribute_reader {
	Iterator iterator;
	Iterator end;

	elements::of<
		code_attribute_reader<Iterator, code_attribute_reader_stage::max_stack>,
		uint16
	>
	operator () () const
	requires (Stage == code_attribute_reader_stage::max_locals) {
		auto cpy = iterator;
		uint16 max_locals = read<uint16>(cpy);
		return { { cpy, end }, max_locals };
	}

	elements::of<
		code_attribute_reader<Iterator, code_attribute_reader_stage::code>,
		uint16
	>
	operator () () const
	requires (Stage == code_attribute_reader_stage::max_stack) {
		auto cpy = iterator;
		uint16 max_stack = read<uint16>(cpy);
		return { { cpy, end }, max_stack };
	}

	template<typename Handler>
	code_attribute_reader<
		Iterator,
		code_attribute_reader_stage::exception_table
	>
	operator () (Handler&& handler) const
	requires (Stage == code_attribute_reader_stage::code) {
		auto cpy = iterator;
		
		uint32 code_length = read<uint32>(cpy);

		while(code_length > 0) {
			--code_length;

			uint8 first_byte = read<uint8>(cpy);

			switch (first_byte) {
			}
		}
	}

};

template<typename Iterator>
code_attribute_reader(Iterator, Iterator) -> code_attribute_reader<Iterator>;
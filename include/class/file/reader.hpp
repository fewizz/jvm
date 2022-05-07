#pragma once

#include "version.hpp"
#include "constant.hpp"
#include "access_flag.hpp"

#include "field/reader.hpp"
#include "method/reader.hpp"

#include <core/meta/elements/of.hpp>
#include <core/array.hpp>
#include <core/equals.hpp>

namespace class_file {

	enum class reader_stage {
		magic, version, constant_pool, access_flags, this_class, super_class,
		interfaces, fields, methods, attributes
	};

	template<typename Iterator, reader_stage Stage = reader_stage::magic>
	struct reader {

	Iterator iterator_;

	elements::of<reader<Iterator, reader_stage::version>, bool>
		operator () () const
		requires (Stage == reader_stage::magic) {
			auto i = iterator_;
			uint32 val = read<uint32, endianness::big>(i);
			bool result = val == 0xCAFEBABE;
			return { { i }, result };
		}

		elements::of<
			reader<Iterator, reader_stage::constant_pool>,
			class_file::version
		>
		operator() () const
		requires (Stage == reader_stage::version) {
			auto i = iterator_;
			auto min = read<uint16, endianness::big>(i);
			auto maj = read<uint16, endianness::big>(i);
			return { { i }, { min, maj } };
		}

		template<typename Handler>
		reader<Iterator, reader_stage::access_flags>
		operator() (Handler&& handler) const
		requires (Stage == reader_stage::constant_pool) {
			auto i = iterator_;
			uint16 size = read<uint16, endianness::big>(i);

			for(uint16 x = 1; x < size; ++x) {
				uint8 tag = *i++;

				switch (tag) {
					case constant::utf8::tag : {
						uint16 len = read<uint16, endianness::big>(i);
						handler(constant::utf8{ i, len }, x);
						i += len;
						break;
					}
					case constant::int32::tag : {
						int32 value = read<int32, endianness::big>(i);
						handler(constant::int32{ value }, x);
						break;
					}
					case constant::float32::tag : {
						float value = read<float, endianness::big>(i);
						handler(constant::float32{ value }, x);
						break;
					}
					case constant::int64::tag : {
						int64 val = read<uint64, endianness::big>(i);
						handler(constant::int64{ val }, x);
						++x;
						handler(constant::skip{}, x);
						break;
					}
					case constant::float64::tag : {
						double value = read<double, endianness::big>(i);
						handler(constant::float64{ value }, x);
						++x;
						handler(constant::skip{}, x);
						break;
					}
					case constant::_class::tag : {
						uint16 name_index = read<uint16, endianness::big>(i);
						handler(constant::_class{ name_index }, x);
						break;
					}
					case constant::string::tag : {
						uint16 index = read<uint16, endianness::big>(i);
						handler(constant::string{ index }, x);
						break;
					}
					case constant::field_ref::tag : {
						uint16 class_index = read<uint16, endianness::big>(i);
						uint16 nat_index = read<uint16, endianness::big>(i);
						handler(
							constant::field_ref{ class_index, nat_index },
							x
						);
						break;
					}
					case constant::method_ref::tag : {
						uint16 class_index = read<uint16, endianness::big>(i);
						uint16 nat_index = read<uint16, endianness::big>(i);
						handler(
							constant::method_ref{ class_index, nat_index },
							x
						);
						break;
					}
					case constant::interface_method_ref::tag : {
						uint16 class_index = read<uint16, endianness::big>(i);
						uint16 nat_index = read<uint16, endianness::big>(i);
						handler(
							constant::interface_method_ref {
								class_index,
								nat_index
							},
							x
						);
						break;
					}
					case constant::name_and_type::tag : {
						uint16 name_index = read<uint16, endianness::big>(i);
						uint16 desc_index = read<uint16, endianness::big>(i);
						handler(
							constant::name_and_type {
								name_index,
								desc_index
							},
							x);
						break;
					}
					case constant::method_handle::tag : {
						uint8 kind = *i++;
						uint16 index = read<uint16, endianness::big>(i);
						handler(constant::method_handle{ kind, index }, x);
						break;
					}
					case constant::method_type::tag : {
						uint16 desc_index = read<uint16, endianness::big>(i);
						handler(constant::method_type{ desc_index }, x);
						break;
					}
					case constant::dynamic::tag : {
						uint16 method_attr_index {
							read<uint16, endianness::big>(i)
						};
						uint16 name_and_type_index {
							read<uint16, endianness::big>(i)
						};
						handler(
							constant::dynamic {
								method_attr_index, name_and_type_index
							},
							x
						);
						break;
					}
					case constant::invoke_dynamic::tag : {
						uint16 method_attr_index {
							read<uint16, endianness::big>(i)
						};
						uint16 name_and_type_index {
							read<uint16, endianness::big>(i)
						};
						handler(
							constant::invoke_dynamic {
								method_attr_index,
								name_and_type_index
							},
							x);
						break;
					}
					case constant::module::tag : {
						uint16 name_index = read<uint16, endianness::big>(i);
						handler(constant::module{ name_index }, x);
						break;
					}
					case constant::package::tag : {
						uint16 name_index = read<uint16, endianness::big>(i);
						handler(constant::package{ name_index }, x);
						break;
					}
					default: handler(constant::unknown{ tag }, x);
				}

			}

			return { i };
		}


		uint16 entries_count() const
		requires (Stage == reader_stage::constant_pool) {
			uint16 constant_pool_size = 0;
			(*this)([&](auto, uint16) { ++constant_pool_size; });
			return constant_pool_size;
		}

		elements::of<
			reader<Iterator, reader_stage::this_class>,
			access_flags
		>
		operator () () const
		requires (Stage == reader_stage::access_flags) {
			auto i = iterator_;
			auto flags = (access_flag) read<uint16, endianness::big>(i);
			return { { i }, flags };
		}

		elements::of<
			reader<Iterator, reader_stage::super_class>,
			uint16
		>
		operator () () const
		requires (Stage == reader_stage::this_class) {
			auto i = iterator_;
			auto ind = read<uint16, endianness::big>(i);
			return { { i }, ind };
		}

		elements::of<
			reader<Iterator, reader_stage::interfaces>,
			uint16
		>
		operator () () const
		requires (Stage == reader_stage::super_class) {
			auto i = iterator_;
			auto ind = read<uint16, endianness::big>(i);
			return { { i }, ind };
		}

		template<typename Handler>
		reader<Iterator, reader_stage::fields>
		operator () (Handler&& handler) const
		requires (Stage == reader_stage::interfaces) {
			auto i = iterator_;
			uint16 count = read<uint16, endianness::big>(i);
			for(uint16 x = 0; x < count; ++x) {
				uint16 index = read<uint16, endianness::big>(i);
				handler(index);
			}

			return { i };
		}

		uint16 count() const
		requires (Stage == reader_stage::interfaces) {
			auto i = iterator_;
			return read<uint16, endianness::big>(i);
		}

		uint16 count () const
		requires (Stage == reader_stage::fields) {
			auto i = iterator_;
			return read<uint16, endianness::big>(i);
		}

		template<typename Handler>
		reader<Iterator, reader_stage::methods>
		operator () (Handler&& handler) const
		requires (Stage == reader_stage::fields) {
			auto i = iterator_;
			uint16 count = read<uint16, endianness::big>(i);

			for(uint16 x = 0; x < count; ++x) {
				field::reader<Iterator, field::reader_stage::end> result {
					handler(field::reader{ i })
				};
				i = result.iterator_;
			}

			return { i };
		}

		uint16 count () const
		requires (Stage == reader_stage::methods) {
			auto i = iterator_;
			return read<uint16, endianness::big>(i);
		}

		template<typename Handler>
		reader<Iterator, reader_stage::attributes>
		operator () (Handler&& handler) const
		requires (Stage == reader_stage::methods) {
			auto i = iterator_;
			uint16 count = read<uint16, endianness::big>(i);

			for(uint16 x = 0; x < count; ++x) {
				method::reader<Iterator, method::reader_stage::end> result {
					handler(method::reader{ i })
				};
				i = result.iterator_;
			}

			return { i };
		}

	};

	template<typename Iterator, reader_stage Stage = reader_stage::magic>
	reader(Iterator) -> reader<Iterator, Stage>;

}
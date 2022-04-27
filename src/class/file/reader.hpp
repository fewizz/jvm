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

	template<typename Iterator, reader_stage Stage = reader_stage::magic>
	struct reader {

	Iterator src;

	elements::of<reader<Iterator, reader_stage::version>, bool>
		operator () () const
		requires (Stage == reader_stage::magic) {
			auto cpy = src;

			uint32 val = read<uint32, endianness::big>(cpy);
			bool result = val == 0xCAFEBABE;

			return {
				{ cpy },
				result
			};
		}

		elements::of<
			reader<Iterator, reader_stage::constant_pool>,
			class_file::version
		>
		operator() () const
		requires (Stage == reader_stage::version) {
			auto cpy = src;

			auto min = read<uint16, endianness::big>(cpy);
			auto maj = read<uint16, endianness::big>(cpy);

			return {
				{ cpy },
				{ min, maj }
			};
		}

		template<typename Handler>
		reader<Iterator, reader_stage::access_flags>
		operator() (Handler&& handler) const
		requires (Stage == reader_stage::constant_pool) {
			auto cpy = src;

			uint16 size = read<uint16, endianness::big>(cpy);

			for(uint16 i = 1; i < size; ++i) {
				uint8 tag = *cpy++;

				switch (tag) {
					case constant::utf8::tag : {
						uint16 len = read<uint16, endianness::big>(cpy);
						handler(constant::utf8{ cpy, len });
						cpy += len;
						break;
					}
					case constant::integer::tag : {
						int32 value = read<int32, endianness::big>(cpy);
						handler(constant::integer{ value });
						break;
					}
					case constant::_float::tag : {
						float value = read<float, endianness::big>(cpy);
						handler(constant::_float{ value });
						break;
					}
					case constant::_long::tag : {
						int64 val = read<uint64, endianness::big>(cpy);
						handler(constant::_long{ val });
						++i;
						handler(constant::skip{});
						break;
					}
					case constant::_double::tag : {
						double value = read<double, endianness::big>(cpy);
						handler(constant::_double{ value });
						++i;
						handler(constant::skip{});
						break;
					}
					case constant::_class::tag : {
						uint16 name_index = read<uint16, endianness::big>(cpy);
						handler(constant::_class{ name_index });
						break;
					}
					case constant::string::tag : {
						uint16 index = read<uint16, endianness::big>(cpy);
						handler(constant::string{ index });
						break;
					}
					case constant::fieldref::tag : {
						uint16 class_index = read<uint16, endianness::big>(cpy);
						uint16 nat_index = read<uint16, endianness::big>(cpy);
						handler(constant::fieldref{ class_index, nat_index });
						break;
					}
					case constant::methodref::tag : {
						uint16 class_index = read<uint16, endianness::big>(cpy);
						uint16 nat_index = read<uint16, endianness::big>(cpy);
						handler(constant::methodref{ class_index, nat_index });
						break;
					}
					case constant::interface_methodref::tag : {
						uint16 class_index = read<uint16, endianness::big>(cpy);
						uint16 nat_index = read<uint16, endianness::big>(cpy);
						handler(constant::interface_methodref {
							class_index,
							nat_index
						});
						break;
					}
					case constant::name_and_type::tag : {
						uint16 name_index = read<uint16, endianness::big>(cpy);
						uint16 desc_index = read<uint16, endianness::big>(cpy);
						handler(constant::name_and_type {
							name_index,
							desc_index
						});
						break;
					}
					case constant::method_handle::tag : {
						uint8 kind = *cpy++;
						uint16 index = read<uint16, endianness::big>(cpy);
						handler(constant::method_handle{ kind, index });
						break;
					}
					case constant::method_type::tag : {
						uint16 desc_index = read<uint16, endianness::big>(cpy);
						handler(constant::method_type{ desc_index });
						break;
					}
					case constant::dynamic::tag : {
						uint16 method_attr_index {
							read<uint16, endianness::big>(cpy)
						};
						uint16 name_and_type_index {
							read<uint16, endianness::big>(cpy)
						};
						handler(constant::dynamic {
							method_attr_index, name_and_type_index
						});
						break;
					}
					case constant::invoke_dynamic::tag : {
						uint16 method_attr_index {
							read<uint16, endianness::big>(cpy)
						};
						uint16 name_and_type_index {
							read<uint16, endianness::big>(cpy)
						};
						handler(constant::invoke_dynamic {
							method_attr_index,
							name_and_type_index
						});
						break;
					}
					case constant::module::tag : {
						uint16 name_index = read<uint16, endianness::big>(cpy);
						handler(constant::module{name_index});
						break;
					}
					case constant::package::tag : {
						uint16 name_index = read<uint16, endianness::big>(cpy);
						handler(constant::package{name_index});
						break;
					}
					default: handler(tag);
				}

			}

			return { cpy };
		}

		elements::of<
			reader<Iterator, reader_stage::this_class>,
			access_flags
		>
		operator () () const
		requires (Stage == reader_stage::access_flags) {
			auto cpy = src;
			auto flags = (access_flag) read<uint16, endianness::big>(cpy);

			return { { cpy }, { flags } };
		}

		elements::of<
			reader<Iterator, reader_stage::super_class>,
			uint16
		>
		operator () () const
		requires (Stage == reader_stage::this_class) {
			auto cpy = src;
			auto ind = read<uint16, endianness::big>(cpy);
			return { { cpy }, ind };
		}

		elements::of<
			reader<Iterator, reader_stage::interfaces>,
			uint16
		>
		operator () () const
		requires (Stage == reader_stage::super_class) {
			auto cpy = src;
			auto ind = read<uint16, endianness::big>(cpy);
			return { { cpy }, ind };
		}

		template<typename Handler>
		reader<Iterator, reader_stage::fields>
		operator () (Handler&& handler) const
		requires (Stage == reader_stage::interfaces) {
			auto cpy = src;

			uint16 count = read<uint16, endianness::big>(cpy);

			for(uint16 i = 0; i < count; ++i) {
				uint16 index = read<uint16, endianness::big>(cpy);
				handler(index);
			}

			return { cpy };
		}

		uint16 count () const
		requires (Stage == reader_stage::fields) {
			auto cpy = src;
			uint16 count = read<uint16, endianness::big>(cpy);
			return count;
		}

		template<typename Handler>
		reader<Iterator, reader_stage::methods>
		operator () (Handler&& handler) const
		requires (Stage == reader_stage::fields) {
			auto cpy = src;
			uint16 count = read<uint16, endianness::big>(cpy);

			for(uint16 i = 0; i < count; ++i) {
				field::reader<Iterator, field::reader_stage::end> result {
					handler(field::reader{ cpy })
				};
				cpy = result.src;
			}

			return { cpy };
		}

		uint16 count () const
		requires (Stage == reader_stage::methods) {
			auto cpy = src;
			uint16 count = read<uint16, endianness::big>(cpy);
			return count;
		}

		template<typename Handler>
		reader<Iterator, reader_stage::attributes>
		operator () (Handler&& handler) const
		requires (Stage == reader_stage::methods) {
			auto cpy = src;
			uint16 count = read<uint16, endianness::big>(cpy);

			for(uint16 i = 0; i < count; ++i) {
				method::reader<Iterator, method::reader_stage::end> result {
					handler(method::reader{ cpy })
				};
				cpy = result.src;
			}

			return { cpy };
		}

	};

	template<typename Iterator, reader_stage Stage = reader_stage::magic>
	reader(Iterator) -> reader<Iterator, Stage>;

}
#pragma once

#include "../constant.hpp"

#include <core/read.hpp>
#include <core/meta/elements/one_of.hpp>
#include <core/array.hpp>
#include <core/range/equals.hpp>

namespace class_file::attribute {

	enum class reader_stage {
		info,
		end
	};

	template<typename Iterator, reader_stage Stage = reader_stage::info>
	struct reader {
		Iterator src;

		template<
			typename IndexToUtf8Mapper,
			typename Handler
		>
		reader<Iterator, reader_stage::end>
		operator () (IndexToUtf8Mapper&& mapper, Handler&& handler) const
		requires (Stage == reader_stage::info) {
			auto cpy = src;
			uint16 name_index = read<uint16>(cpy);
			uint32 length = read<uint32>(cpy);
			constant::utf8 name = mapper(name_index);

			//if(range::equals(name, array{ 'C', 'o', 'd', 'e' }) {
			//	handler();
			//}

			cpy += length;

			return { cpy };
		}
	};

	template<typename Iterator>
	reader(Iterator) -> reader<Iterator>;

} // class_file::attribute
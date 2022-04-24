#pragma once

#include "../access_flag.hpp"

#include "../attribute/reader.hpp"

#include <core/meta/elements/of.hpp>
#include <core/read.hpp>

namespace class_file::field {

	enum class reader_stage {
		access_flags,
		name_index,
		descriptor_index,
		attributes,
		end
	};

	template<typename Iterator, reader_stage Stage = reader_stage::access_flags>
	struct reader {
		Iterator src;

		elements::of<reader<Iterator, reader_stage::name_index>, access_flags>
		operator () () const
		requires (Stage == reader_stage::access_flags) {
			auto cpy = src;
			uint16 flags = read<uint16>(cpy);
			return { { cpy }, { (access_flag) flags } };
		}

		elements::of<reader<Iterator, reader_stage::descriptor_index>, uint16>
		operator () () const
		requires (Stage == reader_stage::name_index) {
			auto cpy = src;
			uint16 name_index = read<uint16>(cpy);
			return { { cpy }, { name_index } };
		}

		elements::of<reader<Iterator, reader_stage::attributes>, uint16>
		operator () () const
		requires (Stage == reader_stage::descriptor_index) {
			auto cpy = src;
			uint16 desc_index = read<uint16>(cpy);
			return { { cpy }, { desc_index } };
		}

		template<typename Handler>
		reader<Iterator, reader_stage::end>
		operator () (Handler&& handler) const
		requires (Stage == reader_stage::attributes) {
			auto cpy = src;
			auto count = read<uint16>(cpy);

			while(count > 0) {
				--count;

				attribute::reader<Iterator, attribute::reader_stage::end> end {
					handler(attribute::reader{ cpy })
				};

				cpy = end.src;
			}

			return { cpy };
		}

	};

	template<typename Iterator>
	reader(Iterator) -> reader<Iterator>;

}
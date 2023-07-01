#pragma once

#include "decl/class/bootstrap_methods.hpp"

#include <class_file/attribute/bootstrap_methods/reader.hpp>

#include <list.hpp>
#include <posix/memory.hpp>

template<basic_iterator Iterator>
bootstrap_method read_bootstrap_method(
	class_file::attribute::bootstrap::method::reader<Iterator> bm_read
) {
	auto [reference_index, arguments_count_reader] {
		bm_read.read_reference_index_and_get_arguments_count_reader()
	};
	auto [arguments_count, arguments_reader] {
		arguments_count_reader.read_and_get_arguments_reader()
	};

	::list arguments_indices_raw {
		posix::allocate<class_file::constant::index>(arguments_count)
	};

	arguments_reader.read(
		[&](class_file::constant::index index) {
			arguments_indices_raw.emplace_back(index);
		}
	);

	return {
		reference_index, move(arguments_indices_raw.storage_range())
	};
}

template<typename Iterator>
::bootstrap_methods read_bootstap_methods(
	class_file::attribute::bootstrap_methods::reader<Iterator> reader
) {
		auto [count, bootstrap_methods_reader] {
			reader.read_count_and_get_methods_reader()
		};

		::list bootstrap_methods_raw {
			posix::allocate<bootstrap_method>(count)
		};

		bootstrap_methods_reader.read(
			[&](auto bm_reader) {
				bootstrap_methods_raw.emplace_back(
					read_bootstrap_method(bm_reader)
				);
			}
		);

		return ::bootstrap_methods {
			move(bootstrap_methods_raw.storage_range())
		};
	};
#pragma once

#include "class/bootstrap_methods.hpp"
#include <list.hpp>
#include <class_file/attribute/bootstrap_methods/reader.hpp>

static bootstrap_methods read_bootstap_methods(auto reader) {
	auto [count, bootstrap_methods_reader] {
		reader.read_count_and_get_methods_reader()
	};

	list bootstrap_methods_raw {
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

			list arguments_indices_raw {
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

	return bootstrap_methods{ bootstrap_methods_raw.move_storage_range() };
}
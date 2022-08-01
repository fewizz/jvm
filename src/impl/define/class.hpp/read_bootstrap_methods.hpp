#pragma once

#include "class/bootstrap_method_pool.hpp"

static bootstrap_method_pool read_bootstap_methods(auto reader) {
	using namespace class_file::attribute::bootstrap::method;

	auto [count, methods_reader] {
		reader.read_count_and_get_methods_reader()
	};

	bootstrap_method_pool bootstrap_methods = bootstrap_method_pool{ count };

	methods_reader.read(
		count,
		[&](auto method_reader) {
			auto [reference_index, arguments_count_reader] {
				method_reader
					.read_reference_index_and_get_arguments_count_reader()
			};
			auto [arguments_count, arguments_reader] {
				arguments_count_reader.read_and_get_arguments_reader()
			};

			bootstrap_method_arguments_indices arguments_indices {
				arguments_count
			};

			arguments_reader.read(
				arguments_count,
				[&](argument_index index) {
					arguments_indices.emplace_back(index);
				}
			);

			bootstrap_methods.emplace_back(
				reference_index, move(arguments_indices)
			);
		}
	);

	return move(bootstrap_methods);
}
#pragma once

#include "reference.hpp"
#include "classes.hpp"

#include <posix/io.hpp>
#include <ranges.hpp>
#include <array.hpp>

template<basic_range RootPath, basic_range Name>
optional<posix::memory<>>
try_load_class_file_data_at(RootPath&& root_path, Name&& name) {
	auto null_terminated = ranges {
		root_path, array{u8'/'},
		name, u8".class"s, array{u8'\0'}
	}.concat_view();

	expected<handle<posix::file>, posix::error> possible_file
		= null_terminated.view_copied_elements_on_stack(
			[&](span<utf8::unit> on_stack) {
				return posix::try_open_file(
					on_stack,
					posix::file_access_mode::binary_read
				);
			}
		);

	if(possible_file.is_unexpected()) {
		return {};
	}

	body<posix::file> f = possible_file.get_expected();

	nuint size = f->set_offset_to_end();
	f->set_offset(0);

	posix::memory<> data = posix::allocate<>(size);

	nuint read_total = 0;
	while(read_total < size) {
		nuint read = f->read_to(
			span{data.iterator() + read_total, size - read_total}
		);
		if(read == 0) break;
		read_total += read;
	}

	return { move(data) };
}
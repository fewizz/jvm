#pragma once

#include "execution/info.hpp"
#include "view_class_file.hpp"
#include "class.hpp"
#include "define/class.hpp"
#include "define/array_class.hpp"

#include <range.hpp>

#include <print/print.hpp>

// only for loading NON-LOADED classes.
// otherwise, use find_or_load_class(name)
template<basic_range Name>
inline _class& load_class(Name&& name) {
	if(info) {
		tabs();
		print::out("loading class ");

		range{ name }.view_copied_elements_on_stack([&](auto on_stack) {
			print::out(on_stack);
			print::out("\n");
		});
	}

	if(range{ name }.starts_with('[')) {
		return define_array_class(name);
	}

	return view_class_file(
		name,
		[&](body<posix::file> f) -> _class& {
			nuint size = f->set_offset_to_end();
			f->set_offset(0);
			auto data = posix::allocate_memory_for<uint8>(size);
			nuint read_total = 0;
			while(read_total < size) {
				nuint read = f->read_to(
					span{data.iterator() + read_total, size - read_total}
				);
				if(read == 0) break;
				read_total += read;
			}
			return define_class(move(data));
		}
	);
}
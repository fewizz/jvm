#pragma once

#include "execution/info.hpp"
#include "view_class_file.hpp"
#include "class.hpp"
#include "define/class.hpp"
#include "define/array_class.hpp"

#include <range.hpp>

#include <posix/io.hpp>

// only for loading NON-LOADED classes.
// otherwise, use find_or_load_class(name)
template<basic_range Name>
inline _class& load_class(Name&& name) {
	if(info) {
		tabs();
		print("loading class ");
	}

	range{ name }.view_copied_elements_on_stack([&](auto on_stack) {
		print(on_stack);
		print("\n");
	});

	if(range{ name }.starts_with('[')) {
		return define_array_class(name);
	}

	return view_class_file(name,
		[&](posix::own_file& f) -> _class& {
			nuint size = f->set_offset_to_end();
			f->set_offset(0);
			auto data = posix::allocate_memory_for<uint8>(size);
			f->read_to(data);
			return define_class(move(data));
		}
	);
}
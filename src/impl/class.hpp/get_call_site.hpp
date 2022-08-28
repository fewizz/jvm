#pragma once

#include "decl/class.hpp"
#include "decl/object/reference.hpp"
#include "decl/execute.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"

#include <class_file/constant.hpp>

#include <memory_list.hpp>

inline reference _class::get_call_site(
	class_file::constant::invoke_dynamic_index index
) {
	if(auto e = trampoline(index); !e.is<elements::none>()) {
		if(!e.is<reference>()) {
			abort();
		}
		return e.get<reference>();
	}

	class_file::constant::invoke_dynamic invoke_dynamic
		= invoke_dynamic_constant(index);
	
	bootstrap_method& bm
		= bootstrap_methods::operator [] (
			invoke_dynamic.bootstrap_method_attr_index
		);

	reference mh = get_method_handle(bm.method_handle_index);

	uint16 args_storage_size
		= bm.arguments_indices.size() * sizeof(stack_entry);
	alignas(stack_entry[]) uint8 args_storage[args_storage_size];
	::memory_list<stack_entry> args {
		memory_span{ args_storage, args_storage_size }
	};

	for(auto index : bm.arguments_indices) {
		constant(index).view([&]<typename Type>(Type v) {
			if constexpr(same_as<Type, class_file::constant::_int>) {
				args.emplace_back(jint{ v.value });
			}
			else if constexpr(same_as<Type, class_file::constant::string>) {
				args.emplace_back(
					get_string((class_file::constant::string_index) index)
				);
			}
			else {
				abort();
			}
		});
	}

	optional<stack_entry> result
		= method_handle_invoke_exact(mh, arguments_span{ args });
	
	if(!result.has_value() || result->is<reference>()) {
		abort();
	}

	reference ref = move(result->get<reference>());
	trampoline(index) = ref;
	return ref;
}
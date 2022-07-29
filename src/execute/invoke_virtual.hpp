#pragma once

#include "./method_ref_index.hpp"
#include "execute.hpp"
#include "execution/info.hpp"
#include "execution/stack_entry.hpp"
#include "class.hpp"
#include "object.hpp"

#include <class_file/constant.hpp>

inline reference invoke_virtual(
	method_ref_index ref_index, _class& c,
	stack_entry* stack, nuint& stack_size
) {
	namespace cf = class_file;
	namespace cc = cf::constant;

	cc::method_ref method_ref { c.method_ref_constant(ref_index) };
	cc::name_and_type nat {
		c.name_and_type_constant(method_ref.name_and_type_index)
	};
	cc::utf8 method_name = c.utf8_constant(nat.name_index);
	cc::utf8 method_desc = c.utf8_constant(nat.descriptor_index);

	if(info) {
		cc::_class _c { c.class_constant(method_ref.class_index) };
		cc::utf8 class_name = c.utf8_constant(_c.name_index);
		tabs(); fputs("invoke_virtual ", stderr);
		fwrite(class_name.data(), 1, class_name.size(), stderr);
		fputc('.', stderr);
		fwrite(method_name.data(), 1, method_name.size(), stderr);
		fwrite(method_desc.data(), 1, method_desc.size(), stderr);
		fputc('\n', stderr);
	}

	uint8 args_count;
	{
		method_with_class virt_mwc = c.get_resolved_method(ref_index);
		args_count = virt_mwc.method().arguments_count();
	}
	++args_count; // this

	reference& ref = stack[stack_size - args_count].get<reference>();
	optional<_class&> c0 = ref.object()._class();
	optional<method&> m0{};

	while(true) {
		if(m0 = c0->try_find_method(method_name, method_desc); m0.has_value()) {
			break;
		}
		if(!c0->has_super_class()) {
			break;
		}
		c0 = c0->super_class();
	}

	if(!m0.has_value()) {
		nuint index = 0;
		c.for_each_maximally_specific_superinterface_method(
			method_name, method_desc,
			[&](method_with_class mwc) {
				if(index++ == 0) {
					m0 = mwc.method();
					c0 = mwc._class();
					return;
				}
				fputs("more than one maximally-specific", stderr);
				abort();
			}
		);
	}

	if(!m0.has_value()) {
		fputs("couldn't find method", stderr); abort();
	}

	stack_size -= args_count;
	expected<stack_entry, reference> result = execute(
		method_with_class{ m0.value(), c0.value() },
		arguments_container{ stack + stack_size, args_count }
	);

	if(result.is_unexpected()) {
		return result.get_unexpected();
	}

	if(!result.get_expected().is<jvoid>()) {
		stack[stack_size++] = result;
	}

	return {};
}
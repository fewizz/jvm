#include "execute.hpp"
#include "execution/info.hpp"
#include "execution/stack.hpp"
#include "class.hpp"
#include "object.hpp"

#include <class_file/constant.hpp>

inline void invoke_virtual(
	class_file::constant::method_ref_index ref_index, _class& c, stack& stack
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
		args_count = virt_mwc.method().parameters_count();
	}
	++args_count; // this

	reference& ref = stack[stack.size() - args_count].get<reference>();
	optional<_class&> c0 = ref.object()._class();
	optional<method&> m{};

	while(true) {
		if(m = c0->try_find_method(method_name, method_desc); m.has_value()) {
			break;
		}
		if(!c0->has_super_class()) {
			break;
		}
		c0 = c0->super_class();
	}

	if(!m.has_value()) {
		nuint index = 0;
		c.for_each_maximally_specific_superinterface_method(
			method_name, method_desc,
			[&](method_with_class mwc) {
				if(index == 0) {
					m = mwc.method();
					c0 = mwc._class();
					return;
				}
				index++;
			}
		);
	}

	if(!m.has_value()) {
		fputs("couldn't find method", stderr); abort();
	}

	stack_entry result = execute(
		method_with_class{ m.value(), c0.value() },
		arguments_span{ stack.begin() + stack.size() - args_count, args_count }
	);

	while(args_count > 0) {
		--args_count;
		stack.pop_back();
	}

	if(!result.is<jvoid>()) {
		stack.emplace_back(move(result));
	}
}
#include "decl/parameters_count.hpp"
#include "decl/execute.hpp"
#include "decl/execution/info.hpp"
#include "decl/execution/stack.hpp"
#include "decl/class.hpp"
#include "decl/object.hpp"

#include <class_file/constant.hpp>

inline void invoke_interface(
	class_file::constant::interface_method_ref_index ref_index,
	_class& c, stack& stack
) {
	namespace cf = class_file;
	namespace cc = cf::constant;

	/*cc::interface_method_ref method_ref_info =
		c.interface_method_ref_constant(ref_index);
	cc::name_and_type name_and_type_info =
		c.name_and_type_constant(method_ref_info.name_and_type_index);

	auto name = c.utf8_constant(name_and_type_info.name_index);
	auto desc = c.utf8_constant(name_and_type_info.descriptor_index);

	if(info) {
		cc::_class class_info {
			c.class_constant(method_ref_info.interface_index)
		};
		auto class_name = c.utf8_constant(class_info.name_index);
		tabs(); fputs("invoke_interface ", stderr);
		fwrite(class_name.elements_ptr(), 1, class_name.size(), stderr);
		fputc('.', stderr);
		fwrite(name.elements_ptr(), 1, name.size(), stderr);
		fwrite(desc.elements_ptr(), 1, desc.size(), stderr);
		fputc('\n', stderr);
	}

	optional<method&> m0{};
	optional<_class&> c0 = stack[stack.size() - args_count]
		.get<reference>().object()._class();

	while(true) {
		if(m0 = c0->try_find_method(name, desc); m0.has_value()) {
			break;
		}
		if(!c0->has_super_class()) {
			break;
		}
		c0 = c0->super_class();
	}

	if(!m0.has_value()) {
		nuint index = 0;
		c.for_each_maximally_specific_super_interface_method(
			name, desc,
			[&](method& m) {
				if(index++ == 0) {
					m0 = m.method();
					c0 = m._class();
					return;
				}
				fputs(
					"more than one maximally-specific interface method", stderr
				);
				abort();
			}
		);
	}

	if(!m0.has_value()) {
		fputs("couldn't find method", stderr); abort();
	}

	stack_entry result = execute(
		method_with_class{ m0.value(), c0.value() },
		arguments_span{ stack.begin() + stack.size() - args_count, args_count }
	);

	while(args_count > 0) {
		--args_count;
		stack.pop_back();
	}

	if(!result.is<jvoid>()) {
		stack.emplace_back(move(result));
	}*/
	abort();
}
#include "execute.hpp"
#include "execution/info.hpp"
#include "execution/stack.hpp"
#include "class.hpp"
#include "method.hpp"

inline void invoke_static(
	class_file::constant::method_ref_index ref_index, _class& c, stack& stack
) {
	namespace cc = class_file::constant;

	cc::method_ref method_ref = c.method_ref_constant(ref_index);
	cc::name_and_type nat {
		c.name_and_type_constant(method_ref.name_and_type_index)
	};
	cc::utf8 method_desc = c.utf8_constant(nat.descriptor_index);

	if(info) {
		tabs(); fputs("invoke_static ", stderr);
		cc::_class _c = c.class_constant(method_ref.class_index);
		cc::utf8 class_name = c.utf8_constant(_c.name_index);
		fwrite(class_name.data(), 1, class_name.size(), stderr);
		fputc('.', stderr);
		auto method_name = c.utf8_constant(nat.name_index);
		fwrite(method_name.data(), 1, method_name.size(), stderr);
		fwrite(method_desc.data(), 1, method_desc.size(), stderr);
		fputc('\n', stderr);
	}

	method_with_class wic = c.get_static_method(ref_index);

	auto args_count = wic.method().arguments_count();

	stack_entry result = execute(
		wic,
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
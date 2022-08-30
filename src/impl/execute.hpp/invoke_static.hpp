#include "decl/execute.hpp"
#include "decl/execution/info.hpp"
#include "decl/execution/stack.hpp"
#include "decl/class.hpp"
#include "decl/method.hpp"

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
		fwrite(class_name.elements_ptr(), 1, class_name.size(), stderr);
		fputc('.', stderr);
		auto method_name = c.utf8_constant(nat.name_index);
		fwrite(method_name.elements_ptr(), 1, method_name.size(), stderr);
		fwrite(method_desc.elements_ptr(), 1, method_desc.size(), stderr);
		fputc('\n', stderr);
	}

	method& m = c.get_static_method(ref_index);

	auto args_count = m.parameters_count();

	optional<stack_entry> result = execute(
		m,
		arguments_span {
			stack.iterator() + stack.size() - args_count,
			args_count
		}
	);

	stack.pop_back(args_count);

	result.if_has_value([&](stack_entry& value) {
		stack.emplace_back(move(value));
	});
}
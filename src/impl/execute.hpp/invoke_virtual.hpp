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
		fwrite(class_name.elements_ptr(), 1, class_name.size(), stderr);
		fputc('.', stderr);
		fwrite(method_name.elements_ptr(), 1, method_name.size(), stderr);
		fwrite(method_desc.elements_ptr(), 1, method_desc.size(), stderr);
		fputc('\n', stderr);
	}

	instance_method_index index =
		c.get_resolved_instance_method_index(ref_index);
	
	uint8 args_count;
	{
		_class& referenced_class =
			c.get_class(c.method_ref_constant(ref_index).class_index);
		method& m0 = referenced_class.instance_methods()[index];
		args_count = m0.parameters_count();
	}

	++args_count; // this

	reference& ref = stack[stack.size() - args_count].get<reference>();

	method& m = ref->_class().instance_methods()[index];

	stack_entry result = execute(
		m,
		arguments_span {
			stack.iterator() + stack.size() - args_count, args_count
		}
	);

	while(args_count > 0) {
		--args_count;
		stack.pop_back();
	}

	if(!result.is<jvoid>()) {
		stack.emplace_back(move(result));
	}
}
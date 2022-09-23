#include "decl/class.hpp"
#include "decl/method.hpp"

#include <class_file/constant.hpp>

#include <loop_action.hpp>

inline method& _class::get_resolved_method(
	class_file::constant::method_ref_index ref_index
) {
	if(auto& t = trampoline(ref_index); t.has_value()) {
		if(!t.is<method&>()) {
			abort();
		}
		return t.get<method&>();
	}

	namespace cc = class_file::constant;

	cc::method_ref method_ref = method_ref_constant(ref_index);
	method& m = resolve_method(method_ref);
	trampoline(ref_index) = m;
	return m;
}
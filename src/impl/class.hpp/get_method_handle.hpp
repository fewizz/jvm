#include "decl/class.hpp"

#include "decl/lib/java/lang/invoke/method_handle.hpp"

#include <class_file/constant.hpp>

inline reference _class::get_method_handle(
	class_file::constant::method_handle_index index
) {
	if(auto e = trampoline(index); !e.is<elements::none>()) {
		if(!e.is<reference>()) {
			abort();
		}
		return e.get<reference>();
	}

	class_file::constant::method_handle mh = method_handle_constant(index);

	using behavior_kind = class_file::constant::method_handle::behavior_kind;

	switch (mh.kind) {
		case behavior_kind::invoke_static: {
			class_file::constant::method_ref ref = method_ref_constant(
				(class_file::constant::method_ref_index) mh.reference_index
			);
			_class& c = get_class(ref.class_index);
			auto nat = name_and_type_constant(ref.name_and_type_index);
			auto name = utf8_constant(nat.name_index);
			auto desc = utf8_constant(nat.descriptor_index);
			method& m = c.declared_methods().find(name, desc);
			return create_method_handle_invoke_static(m);
		}
		default: abort();
	}
}
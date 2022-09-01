#include "execution/info.hpp"
#include "execution/stack.hpp"
#include "class.hpp"

#include <class_file/constant.hpp>

inline void ldc(
	class_file::constant::index const_index, _class& c, stack& stack
) {
	if(info) {
		tabs(); fputs("ldc ", stderr);
		fprintf(stderr, "%hhd\n", (uint8) const_index);
	}
	constant constant = c.constant(const_index);
	if(constant.is<class_file::constant::_int>()) {
		stack.emplace_back(jint {
			constant.get<class_file::constant::_int>().value
		});
	} else
	if(constant.is<class_file::constant::_float>()) {
		stack.emplace_back(jfloat {
			constant.get<class_file::constant::_float>().value
		});
	} else
	if(constant.is<class_file::constant::string>()) {
		stack.emplace_back(c.get_string(
			class_file::constant::string_index{ const_index }
		));
	} else
	if(constant.is<class_file::constant::_class>()) {
		stack.emplace_back(c.get_resolved_class(
			class_file::constant::class_index{ const_index }
		).instance());
	}
	else {
		fputs("unknown constant", stderr); abort();
	}
}

inline void ldc_2_w(
	class_file::constant::index const_index, _class& c, stack& stack
) {
	if(info) {
		tabs(); fputs("ldc_2_w ", stderr);
		fprintf(stderr, "%hd\n", (uint16) const_index);
	}
	constant constant = c.constant(const_index);
	if(constant.is<class_file::constant::_long>()) {
		stack.emplace_back(jlong {
			constant.get<class_file::constant::_long>().value
		});
	} else
	if(constant.is<class_file::constant::_double>()) {
		stack.emplace_back(jdouble {
			constant.get<class_file::constant::_double>().value
		});
	}
	else {
		fputs("unknown constant", stderr); abort();
	}
}
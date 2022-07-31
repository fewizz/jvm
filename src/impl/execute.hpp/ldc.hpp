#include "execution/info.hpp"
#include "execution/stack_entry.hpp"
#include "class.hpp"

#include <class_file/constant.hpp>

inline void ldc(
	class_file::constant::index const_index, _class& c,
	stack_entry* stack, nuint& stack_size
) {
	if(info) {
		tabs(); fputs("ldc ", stderr);
		fprintf(stderr, "%hhd\n", (uint8) const_index);
	}
	const_pool_entry constatnt = c.constant(const_index);
	if(constatnt.is<class_file::constant::_int>()) {
		stack[stack_size++] = jint {
			constatnt.get<class_file::constant::_int>().value
		};
	} else
	if(constatnt.is<class_file::constant::_float>()) {
		stack[stack_size++] = jfloat {
			constatnt.get<class_file::constant::_float>().value
		};
	} else
	if(constatnt.is<class_file::constant::string>()) {
		stack[stack_size++] = c.get_string(
			class_file::constant::string_index{ const_index }
		);
	} else
	if(constatnt.is<class_file::constant::_class>()) {
		stack[stack_size++] = c.get_class(
			class_file::constant::class_index{ const_index }
		).instance();
	}
	else {
		fputs("unknown constant", stderr); abort();
	}
}

inline void ldc_2_w(
	class_file::constant::index const_index, _class& c,
	stack_entry* stack, nuint& stack_size
) {
	if(info) {
		tabs(); fputs("ldc_2_w ", stderr);
		fprintf(stderr, "%hd\n", (uint16) const_index);
	}
	const_pool_entry constatnt = c.constant(const_index);
	if(constatnt.is<class_file::constant::_long>()) {
		stack[stack_size++] = jlong {
			constatnt.get<class_file::constant::_long>().value
		};
	} else
	if(constatnt.is<class_file::constant::_double>()) {
		stack[stack_size++] = jdouble {
			constatnt.get<class_file::constant::_double>().value
		};
	}
	else {
		fputs("unknown constant", stderr); abort();
	}
}
#include "execution/info.hpp"
#include "execution/stack.hpp"
#include "class.hpp"

#include <class_file/constant.hpp>

inline void ldc(
	class_file::constant::index const_index, _class& c
) {
	if(info) {
		tabs();
		print("ldc ");
		print((uint8) const_index);
		print("\n");
	}
	constant constant = c.constant(const_index);
	if(constant.is<class_file::constant::_int>()) {
		stack.emplace_back(int32 {
			constant.get<class_file::constant::_int>().value
		});
	} else
	if(constant.is<class_file::constant::_float>()) {
		stack.emplace_back(float {
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
		posix::std_err.write_from(c_string{ "unknown constant\n" });
		abort();
	}
}

inline void ldc_2_w(
	class_file::constant::index const_index, _class& c
) {
	if(info) {
		tabs();
		print("ldc_2_w ");
		print((uint16) const_index);
		print("\n");
	}
	constant constant = c.constant(const_index);
	if(constant.is<class_file::constant::_long>()) {
		stack.emplace_back(int64 {
			constant.get<class_file::constant::_long>().value
		});
	} else
	if(constant.is<class_file::constant::_double>()) {
		stack.emplace_back(double {
			constant.get<class_file::constant::_double>().value
		});
	}
	else {
		posix::std_err.write_from(c_string{ "unknown constant\n" });
		abort();
	}
}
#include "execution/info.hpp"
#include "execution/stack.hpp"
#include "class.hpp"

#include <class_file/constant.hpp>

inline optional<reference> try_ldc(
	class_file::constant::index const_index, c& c
) {
	if(info) {
		tabs();
		print::out("ldc ", (uint8) const_index, "\n");
	}
	constant constant = c.constant(const_index);
	if(constant.is_same_as<class_file::constant::_int>()) {
		stack.emplace_back(int32 {
			constant.get_same_as<class_file::constant::_int>().value
		});
		return {};
	} else
	if(constant.is_same_as<class_file::constant::_float>()) {
		stack.emplace_back(float {
			constant.get_same_as<class_file::constant::_float>().value
		});
		return {};
	} else
	if(constant.is_same_as<class_file::constant::string>()) {
		expected<reference, reference> possible_string =
			c.try_get_string(
				class_file::constant::string_index{ const_index }
			);
		if(possible_string.is_unexpected()) {
			return possible_string.move_unexpected();
		}
		stack.emplace_back(possible_string.move_expected());
		return {};
	} else
	if(constant.is_same_as<class_file::constant::_class>()) {
		expected<::c&, reference> possible_c
			= c.try_get_resolved_class(
				class_file::constant::class_index{ const_index }
			);
		
		if(possible_c.is_unexpected()) {
			return possible_c.move_unexpected();
		}
		stack.emplace_back(possible_c.get_expected().object());
		return {};
	}
	else {
		print::err("unknown constant\n");
		posix::abort();
	}
}

inline void ldc_2_w(
	class_file::constant::index const_index, c& c
) {
	if(info) {
		tabs();
		print::out("ldc_2_w ", (uint16) const_index, "\n");
	}
	constant constant = c.constant(const_index);
	if(constant.is_same_as<class_file::constant::_long>()) {
		stack.emplace_back(int64 {
			constant.get_same_as<class_file::constant::_long>().value
		});
	} else
	if(constant.is_same_as<class_file::constant::_double>()) {
		stack.emplace_back(double {
			constant.get_same_as<class_file::constant::_double>().value
		});
	}
	else {
		print::err("unknown constant\n");
		posix::abort();
	}
}
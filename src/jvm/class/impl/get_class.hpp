#pragma once

#include "../../field/declaration.hpp"
#include "../../classes/find_or_load.hpp"
#include "class/file/descriptor/reader.hpp"

_class& _class::get_class(uint16 class_index) {
	if(auto& t = trampoline(class_index); !t.is<elements::none>()) {
		return t.get<_class&>();
	}

	using namespace class_file;
	auto name = utf8_constant(class_constant(class_index).name_index);
	_class& c = find_or_load_class(name);
	trampoline(class_index) = c;
	return c;
}
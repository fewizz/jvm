#pragma once

#include "../field.hpp"
#include "../classes.hpp"
#include "class/file/descriptor/reader.hpp"

_class& _class::get_class(uint16 class_index) {
	if(auto& t = trampoline(class_index); !t.is<decltype(nullptr)>()) {
		return t.get<_class&>();
	}

	using namespace class_file;
	auto name = utf8_constant(class_constant(class_index).name_index);
	_class& c = find_or_load(name);
	trampoline(class_index) = c;
	return c;
}
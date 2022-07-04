#pragma once

#include "../../field/decl.hpp"
#include "../../classes/find_or_load.hpp"
#include "class/file/descriptor/reader.hpp"

_class& _class::get_class(uint16 class_index) {
	if(auto& t = trampoline(class_index); !t.is<elements::none>()) {
		if(!t.is<_class&>()) {
			fputs("invalid const pool entry", stderr);
			abort();
		}
		return t.get<_class&>();
	}

	namespace cc = class_file::constant;

	cc::utf8 name = utf8_constant(class_constant(class_index).name_index);
	_class& c = find_or_load_class(name);
	trampoline(class_index) = c;
	return c;
}
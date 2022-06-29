#pragma once

#include "../../class/member/with_class/declaration.hpp"
#include "class/file/constant.hpp"

struct _class;
struct method;

struct method_with_class : class_member_with_class {
	using base_type = class_member_with_class;

	method_with_class(method& method, ::_class& _class) :
		base_type{ (::class_member&) method, _class }
	{}

	::method& method() {
		return (::method&) class_member();
	}

};
#pragma once

#include "class/member.hpp"

#include <class_file/constant.hpp>

struct _class;

struct class_member_with_class {
private:
	::class_member& class_member_;
	_class& _class_;
public:

	class_member_with_class(
		::class_member& class_member, ::_class& _class
	) :
		class_member_{ class_member },
		_class_{ _class }
	{}

	const ::class_member& class_member() const & { return class_member_; }
	      ::class_member& class_member()       & { return class_member_; }

	const ::_class& _class() const & { return _class_; }
	      ::_class& _class()       & { return _class_; }

	const ::_class* class_ptr() const & { return &_class_; }
	      ::_class* class_ptr()       & { return &_class_; }

	class_file::constant::utf8 descriptor();
	class_file::constant::utf8 name();
};
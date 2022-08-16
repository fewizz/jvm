#pragma once

#include <class_file/access_flag.hpp>
#include <class_file/constant.hpp>

#include <optional.hpp>

struct _class;

struct class_member {
private:
	optional<_class&>                              class_;
	class_file::access_flags                       access_flags_;
	class_file::constant::utf8                     name_;
	class_file::constant::utf8                     desc_;

	friend _class;
public:

	class_member(
		class_file::access_flags               access_flags,
		class_file::constant::utf8             name,
		class_file::constant::utf8             desc
	) :
		access_flags_{ access_flags },
		name_        { name   },
		desc_        { desc   }
	{}

	const ::_class& _class() const { return class_.value(); }
	      ::_class& _class()       { return class_.value(); }

	class_file::constant::utf8 name() const {
		return name_;
	};

	class_file::constant::utf8 descriptor() const {
		return desc_;
	}

	class_file::access_flags access_flags() const {
		return access_flags_;
	}

	bool is_static() const {
		return access_flags()._static();
	}

};
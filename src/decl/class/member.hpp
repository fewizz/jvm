#pragma once

#include "./class/find_by_name_and_descriptor_extension.hpp"

#include <class_file/access_flag.hpp>
#include <class_file/constant.hpp>

#include <optional.hpp>
#include <range.hpp>

struct _class;

struct class_member {
protected:
	optional<_class&> class_;
	const class_file::access_flags access_flags_;
	const class_file::constant::utf8 name_;
	const class_file::constant::utf8 desc_;

	friend _class;
public:

	class_member(
		class_file::access_flags   access_flags,
		class_file::constant::utf8 name,
		class_file::constant::utf8 desc
	) :
		access_flags_{ access_flags },
		name_        { name         },
		desc_        { desc         }
	{}

	const ::_class& _class() const { return class_.get(); }
	      ::_class& _class()       { return class_.get(); }

	class_file::constant::utf8 name() const {
		return name_;
	};

	class_file::constant::utf8 descriptor() const {
		return desc_;
	}

	class_file::access_flags access_flags() const {
		return access_flags_;
	}

	bool is_static() const { return access_flags_._static; }

	bool is_public() const { return access_flags_._public; }
	bool is_protected() const { return access_flags_._protected; }
	bool is_private() const { return access_flags_.super_or_synchronized; }

	bool has_default_access() const {
		return !is_public() && !is_protected() && !is_private();
	}

	span<const char> package() const {
		optional<uint16> possible_slash_index
			= name().try_find_index_of_last_satisfying([](char ch) {
				return ch == '/';
			});
		if(possible_slash_index.has_value()) {
			uint16 slash_index = possible_slash_index.get();
			return { name().iterator(), slash_index };
		}
		else {
			return {};
		}
	}

	template<basic_range Name, basic_range Descriptor>
	bool has_name_and_descriptor_equal_to(
		Name&& name, Descriptor&& desc
	) const {
		return
			this->name().has_equal_size_and_elements(name) &&
			this->descriptor().has_equal_size_and_elements(desc);
	}

	template<basic_range Name>
	bool has_name_equal_to(
		Name&& name
	) const {
		return this->name().has_equal_size_and_elements(name);
	}

};
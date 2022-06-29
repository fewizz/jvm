#pragma once

#include "../../name_index.hpp"
#include "../../descriptor_index.hpp"
#include "class/file/access_flag.hpp"

struct class_member {
private:
	class_file::access_flags access_flags_;
	name_index               name_index_;
	descriptor_index         desc_index_;
public:

	class_member(
		class_file::access_flags access_flags,
		name_index               name_index,
		descriptor_index         desc_index
	) :
		access_flags_{ access_flags },
		name_index_  { name_index   },
		desc_index_  { desc_index   }
	{}

	name_index name_index() const {
		return name_index_;
	};

	descriptor_index descriptor_index() const {
		return desc_index_;
	}

	class_file::access_flags access_flags() const {
		return access_flags_;
	}

	inline bool is_static() const {
		return access_flags().get(class_file::access_flag::_static);
	}
};
#pragma once

#include <class_file/access_flag.hpp>
#include <class_file/constant.hpp>

struct class_member {
private:
	class_file::access_flags access_flags_;
	class_file::constant::name_index               name_index_;
	class_file::constant::descriptor_index         desc_index_;
public:

	class_member(
		class_file::access_flags               access_flags,
		class_file::constant::name_index       name_index,
		class_file::constant::descriptor_index desc_index
	) :
		access_flags_{ access_flags },
		name_index_  { name_index   },
		desc_index_  { desc_index   }
	{}

	class_file::constant::name_index name_index() const {
		return name_index_;
	};

	class_file::constant::descriptor_index descriptor_index() const {
		return desc_index_;
	}

	class_file::access_flags access_flags() const {
		return access_flags_;
	}

	bool is_static() const {
		return access_flags()._static();
	}

};
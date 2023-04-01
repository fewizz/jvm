#pragma once

#include "class.hpp"

template<basic_range Name>
inline _class& load_class(Name&& name, reference class_loader);

template<basic_range Name, basic_range RootPath>
inline optional<posix::memory_for_range_of<unsigned char>>
try_load_class_file_data_at(RootPath&& root_path, Name&& name);

/* The process of loading and creating the nonarray class or interface C denoted
by N using the bootstrap class loader is as follows. */
template<basic_range Name>
inline _class&
load_non_array_class_by_bootstrap_class_loader(
	Name&& name
);

/* The process of loading and creating the nonarray class or interface C denoted
   by N using a user-defined class loader L is as follows. */
template<basic_range Name>
inline _class& load_non_array_class_by_user_class_loader(
	Name&& name, reference l_ref
);

/* The following steps are used to create the array class C denoted by the name
   N in association with the class loader L. L may be either the bootstrap class
   loader or a user-defined class loader. */
template<basic_range Name>
inline _class&
load_array_class(
	Name&& name, reference l_ref
);

template<basic_range Name>
inline _class& load_class(Name&& name, reference class_loader) {
	if(range{name}.starts_with('[')) {
		return load_array_class(forward<Name>(name), move(class_loader));
	}

	return class_loader.is_null() ?
		load_non_array_class_by_bootstrap_class_loader(forward<Name>(name))
		:
		load_non_array_class_by_user_class_loader(
			forward<Name>(name), move(class_loader)
		);
}

template<basic_range Name>
inline _class& load_class_by_bootstrap_class_loader(Name&& name) {
	return range{name}.starts_with('[') ?
		load_array_class(forward<Name>(name), reference{})
		:
		load_non_array_class_by_bootstrap_class_loader(forward<Name>(name));
}

template<basic_range Name>
inline _class& load_class(Name&& name) {
	return load_class_by_bootstrap_class_loader(forward<Name>(name));
}
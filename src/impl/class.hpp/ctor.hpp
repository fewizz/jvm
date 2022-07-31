#include "class.hpp"

inline _class::_class(
	const_pool&& const_pool,
	span<uint8> bytes, class_file::access_flags access_flags,
	::this_class_index this_class_index, ::super_class_index super_class_index,
	interfaces_indices_container&& interfaces,
	instance_fields_container&& instance_fields,
	static_fields_container&& static_fields,
	methods_container&& methods,
	::is_array_class is_array_class,
	::is_primitive_class is_primitive_class
) :
	::const_pool       { move(const_pool)      },
	::trampoline_pool  { ::const_pool::size()  },
	bytes_             { bytes                 },
	access_flags_      { access_flags          },
	this_class_index_  { this_class_index      },
	super_class_index_ { super_class_index     },
	interfaces_        { move(interfaces)      },
	instance_fields_   { move(instance_fields) },
	static_fields_     { move(static_fields)   },
	methods_           { move(methods)         },
	is_array_class_    { is_array_class        },
	is_primitive_class_{ is_primitive_class    }
{}
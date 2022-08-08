#include "class.hpp"

inline _class::_class(
	const_pool&& const_pool,
	trampoline_pool&& trampoline_pool,
	bootstrap_method_pool&& bootstrap_method_pool,
	memory_span bytes, class_file::access_flags access_flags,
	::this_class_index this_class_index, ::super_class_index super_class_index,
	interfaces_indices_container&& interfaces,
	fields_container&& fields,
	methods_container&& methods,
	::is_array_class is_array_class,
	::is_primitive_class is_primitive_class
) :
	::const_pool           { move(const_pool)            },
	::trampoline_pool      { move(trampoline_pool)       },
	::bootstrap_method_pool{ move(bootstrap_method_pool) },
	bytes_                 { bytes                       },
	access_flags_          { access_flags                },
	this_class_index_      { this_class_index            },
	super_class_index_     { super_class_index           },
	interfaces_            { move(interfaces)            },
	declared_fields_       { move(fields)                },
	declared_methods_      { move(methods)               },
	is_array_class_        { is_array_class              },
	is_primitive_class_    { is_primitive_class          }
{}
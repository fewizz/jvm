#include "class.hpp"

inline _class::_class(
	constants&&                    constants,
	trampolines&&                  trampolines,
	bootstrap_methods&&            bootstrap_methods,
	memory_span                    bytes,
	class_file::access_flags       access_flags,
	this_name                      this_name,
	super_name                     super_name,
	memory_list<_class&, uint16>&& interfaces,
	declared_fields&&              declared_fields,
	declared_methods&&             declared_methods,
	is_array                       is_array,
	is_primitive                   is_primitive
) :
	::constants            { move(constants)         },
	::trampolines          { move(trampolines)       },
	::bootstrap_methods    { move(bootstrap_methods) },
	bytes_                 { bytes                   },
	access_flags_          { access_flags            },
	this_name_             { this_name               },
	super_name_            { super_name              },
	interfaces_            { move(interfaces)        },
	declared_fields_       { move(declared_fields)   },
	declared_methods_      { move(declared_methods)  },
	is_array_              { is_array                },
	is_primitive_          { is_primitive            }
{}
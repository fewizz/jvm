#pragma once

#include "decl/classes.hpp"
#include "decl/lib/java/lang/object.hpp"

template<basic_range Name>
expected<c&, reference> classes::try_define_array_class(
	Name&& name, j::c_loader* defining_loader
) {
	return lock_or_throw_linkage_error(
		name,
		defining_loader,
		[&] -> expected<c&, reference> {
			auto declared_instance_fields 
				= posix::allocate<
					instance_field, declared_instance_field_index
				>(2);

			// ptr to data
			new (&declared_instance_fields[declared_instance_field_index{0}]) instance_field {
				class_file::access_flags{ class_file::access_flag::_private },
				class_file::constant::utf8{ nullptr, 0 },
				class_file::constant::utf8{ u8"J" }
			};
			// length
			new (&declared_instance_fields[declared_instance_field_index{1}]) instance_field(
				class_file::access_flags{ class_file::access_flag::_private },
				class_file::constant::utf8{ nullptr, 0 },
				class_file::constant::utf8{ u8"I" }
			);

			class_data data{};

			data.emplace_back(posix::allocate(range_size(name)));

			range{ name }.copy_to(data.back().casted<utf8::unit>());

			class_file::constant::utf8 this_name = data.back().casted<utf8::unit>();

			class_file::constant::utf8 descriptor = this_name;

			return emplace_back(
				constants{},
				bootstrap_methods{},
				move(data),
				class_file::access_flags{ class_file::access_flag::_public },
				this_name,
				descriptor,
				class_file::constant::utf8{},
				object_class.get(),
				posix::memory<c*>{},
				posix::memory<static_field, declared_static_field_index>{},
				move(declared_instance_fields),
				posix::memory<static_method, declared_static_method_index>{},
				posix::memory<instance_method, declared_instance_method_index>{},
				optional<method>{},
				is_array_class{ true },
				is_primitive_class{ false },
				defining_loader == nullptr ?
					nullptr_ref
					: reference{ *defining_loader }
			);
		}
	);
}
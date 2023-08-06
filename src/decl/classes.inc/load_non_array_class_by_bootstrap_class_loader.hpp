#pragma once

#include "decl/classes.hpp"
#include "decl/executable_path.hpp"
#include "decl/lib/java/lang/class_not_found_exception.hpp"
#include "decl/lib/java/lang/string.hpp"
#include "decl/try_load_class_file_data_at.hpp"

/* The process of loading and creating the nonarray class or interface C denoted
by N using the bootstrap class loader is as follows. */
template<basic_range Name>
expected<c&, reference>
classes::try_load_non_array_class_by_bootstrap_class_loader(
	Name&& name
) {
	mutex_->lock();
	on_scope_exit unlock_classes_mutex { [&] {
		mutex_->unlock();
	}};

	/* First, the Java Virtual Machine determines whether the bootstrap class
	   loader has already been recorded as an initiating loader of a class or
	   interface denoted by N. If so, this class or interface is C, and no class
	   loading or creation is necessary. */
	optional<c&> loaded_class
		= try_find_class_which_loading_was_initiated_by(
			name, nullptr
		);
	
	if(loaded_class.has_value()) {
		return loaded_class.get();
	}

	if(info) {
		tabs();
		print::out("loading non-array class ");

		range{name}.view_copied_elements_on_stack([&](auto on_stack) {
			print::out(on_stack);
		});

		print::out(" by bootstrap class-loader\n");
	}

	/* Otherwise, the Java Virtual Machine passes the argument N to an
	   invocation of a method on the bootstrap class loader. To load C, the
	   bootstrap class loader locates a purported representation of C in a
	   platform-dependent manner, then asks the Java Virtual Machine to derive a
	   class or interface C denoted by N from the purported representation using
	   the bootstrap class loader, and then to create C, via the algorithm of
	   §5.3.5. */

	optional<posix::memory<>> possible_data =
		ranges{ lib_path.get(), c_string{ u8"/java.base" } }.concat_view()
		.view_copied_elements_on_stack(
			[&](span<utf8::unit> root_path_on_stack) {
				return try_load_class_file_data_at(
					root_path_on_stack,
					name
				);
			}
		);

	if(!possible_data.has_value()) {
		expected<reference, reference> possible_name_ref
			= try_create_string_from_utf8(name);
		if(possible_name_ref.is_unexpected()) {
			return possible_name_ref.move_unexpected();
		}
		reference name_ref = possible_name_ref.move_expected();
		return try_create_class_not_found_exception(move(name_ref)).get();
	}

	auto data = possible_data.move();

	return try_define_class(forward<Name>(name), move(data), nullptr);
}
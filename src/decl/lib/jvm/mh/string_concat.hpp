#pragma once

#include "decl/object.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/string.hpp"

namespace jvm {

	struct string_concat : j::method_handle {
		static inline optional<::c&> c;
		static inline optional<instance_method&> constructor;
		static inline layout::position recipe_field_position;

		using j::method_handle::method_handle;

		j::string& recipe() {
			object& obj = get<reference>(recipe_field_position).object();
			return (j::string&) obj;
		}

		[[nodiscard]] static inline expected<reference, reference>
		try_create(j::method_type& mt, j::string& recipe) {
			return try_create_object(
				constructor.get(),
				mt, recipe
			);
		}

	};

}
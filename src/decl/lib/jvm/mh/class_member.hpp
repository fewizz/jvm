#pragma once

#include "decl/class.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"

#include <optional.hpp>

namespace jvm {

	struct class_member : j::method_handle {
		static inline optional<::c&> c;
		static inline layout::position member_class_position;
		static inline layout::position member_index_position;

		using j::method_handle::method_handle;

		::c& member_c() {
			reference& c_ref = get<reference>(member_class_position);
			j::c& obj = (j::c&) c_ref.object();
			return obj.get_c();
		}

		template<typename IndexType>
		auto& member() {
			IndexType member_index {
				get<uint16>(member_index_position)
			};
			return member_c()[member_index];
		}

	};

}
#pragma once

#include "class/file/access_flag.hpp"
#include "../name_index.hpp"
#include "../descriptor_index.hpp"

#include <core/range_of_value_type_same_as.hpp>
#include <core/meta/type/is_same_as.hpp>
#include <core/meta/elements/of.hpp>

template<typename Handler>
class constants_to_add {
	Handler handler_;
	uint16 count_;

public:

	constants_to_add(uint16 count, Handler&& handler) :
		handler_{ forward<Handler>(handler) },
		count_{ count }
	{}

	decltype(auto) handler() {
		return (handler_);
	}

	uint16 count() { return count_; }

};

template<typename Handler>
constants_to_add(Handler&&, uint16) -> constants_to_add<Handler>;

struct is_constants_to_add : type::predicate_marker {

	template<typename Type>
	static constexpr bool for_type = false;

	template<typename Handler>
	static constexpr bool for_type<constants_to_add<Handler>> = true;

};
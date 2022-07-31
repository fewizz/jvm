#pragma once

#include "class.hpp"
#include "native/function.hpp"
#include "method/with_class.hpp"
#include "alloc.hpp"

#include <core/limited_list.hpp>
#include <core/transform.hpp>
#include <core/concat.hpp>
#include <core/equals.hpp>

static struct native_functoins_container :
	limited_list<native_function, uint32, default_allocator>
{
	using base_type = limited_list<native_function, uint32, default_allocator>;
	using base_type::base_type;

	template<range Name>
	optional<native_function&> try_find(Name name) {
		for(native_function& f : *this) {
			if(equals(f.name(), name)) {
				return { f };
			}
		}
		return elements::none{};
	}

	template<range Name>
	native_function& find(Name name) {
		if(
			optional<native_function&> f {
				try_find(forward<Name>(name))
			};
			f.has_value()
		) {
			return f.value();
		}
		fputs("couldn't find native function in container", stderr);
		abort();
	}

	native_function& find(method_with_class m) {
		transform_view undescored_class_name{ m._class().name(), [](const char c) {
			if(c == '/') return '_';
			return c;
		} };
		concat_view name {
			c_string{ "Java_" },
			undescored_class_name,
			c_string{ "_" },
			m.name()
		};

		return find(name);
	}
} native_functions{ 65536 };
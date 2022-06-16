#pragma once

#include "../declaration.hpp"
#include "../../field/declaration.hpp"

template<typename Name, typename Descriptor, typename Handler>
inline void _class::for_each_maximally_specific_superinterface_method(
	Name&& name, Descriptor&& descriptor, Handler&& handler
) {
	auto check = [&](_class& c) {
		for(auto& m : c.methods()) {
			if(
				equals(m.name(), name) &&
				equals(m.descriptor(), descriptor)
			) {
				handler(m);
				return true;
			}
		}
		return false;
	};
	struct recursive { recursive(decltype(check) check, _class& c) {
		for(auto& i : c.interfaces()) {
			if(check(i)) break;
			recursive{ check, i };
		}
	}};
	recursive{ check, *this };
}
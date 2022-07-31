#include "class.hpp"
#include "field.hpp"

template<typename Name, typename Descriptor, typename Handler>
inline void _class::for_each_maximally_specific_superinterface_method(
	Name&& name, Descriptor&& descriptor, Handler&& handler
) {
	auto check = [&](_class& c) {
		for(auto& m : c.methods()) {
			if(
				equals(this->name(m), name) &&
				equals(this->descriptor(m), descriptor)
			) {
				handler(method_with_class{ m, c });
				return true;
			}
		}
		return false;
	};
	struct recursive { recursive(decltype(check) check, _class& c) {
		for(_class& i : c.interfaces()) {
			if(check(i)) break;
			recursive{ check, i };
		}
	}};
	recursive{ check, *this };
}
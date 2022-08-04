#include "class.hpp"
#include "field.hpp"

#include <core/loop_action.hpp>

template<typename Name, typename Descriptor, typename Handler>
inline void _class::for_each_maximally_specific_superinterface_method(
	Name&& name, Descriptor&& descriptor, Handler&& handler
) {
	auto search_for_method = [&](_class& c) -> optional<method&> {
		for(auto& m : c.methods()) {
			if(
				equals(this->name(m), name) &&
				equals(this->descriptor(m), descriptor) &&
				!m.access_flags()._private() &&
				!m.access_flags()._static()
			) {
				return m;
			}
		}
		return {};
	};
	struct recursive {
		loop_action operator()(decltype(search_for_method) search, _class& c) {
			for(_class& i : c.interfaces()) {
				optional<method&> m = search(i);
				loop_action action;

				// "..., there exists no other maximally-specific superinterface
				// method of C with the specified name and descriptor that is
				// declared in a subinterface of I."
				// thus, not searching in superinterfaces
				if(m.has_value()) {
					action = handler(method_with_class{ m.value(), i });
				}
				// search in superinterfaces
				else {
					action = recursive{ search, i };
				}

				switch (action) {
					case loop_action::stop: return loop_action::stop;
					case loop_action::next: continue;
				}
			}
			return loop_action::next;
		}
	};
	recursive{ search_for_method, *this };
}
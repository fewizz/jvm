#include "decl/class.hpp"
#include "decl/field.hpp"
#include "decl/class/has_name_and_descriptor_equal_to.hpp"

#include <loop_action.hpp>

template<typename Name, typename Descriptor, typename Handler>
inline void _class::for_each_maximally_specific_super_interface_instance_method(
	Name&& name, Descriptor&& descriptor, Handler&& handler
) {
	auto search_for_method = [&](_class& c) -> optional<method&> {
		for(method* m : c.declared_instance_methods()) {
			if(
				has_name_and_descriptor_equal_to{ name, descriptor }(m) &&
				!m->access_flags()._private
			) {
				return *m;
			}
		}
		return {};
	};
	struct recursive {
		loop_action operator()(
			Handler&& handler, decltype(search_for_method) search, _class& c
		) {
			for(_class* i : c.declared_interfaces()) {
				loop_action action;

				optional<method&> m = search(*i);

				// "..., there exists no other maximally-specific superinterface
				// method of C with the specified name and descriptor that is
				// declared in a subinterface of I."
				// thus, not searching in farther superinterfaces
				if(m.has_value()) {
					action = handler(m.value());
				}
				// search in superinterfaces
				else {
					action = recursive{}(forward<Handler>(handler), search, i);
				}

				switch (action) {
					case loop_action::stop: return loop_action::stop;
					case loop_action::next: continue;
				}
			}
			return loop_action::next;
		}
	};
	recursive{}(forward<Handler>(handler), search_for_method, *this);
}
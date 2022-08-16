#include "decl/class.hpp"

#include <loop_action.hpp>

template<typename Handler>
inline void _class::for_each_super_interface(Handler &&handler) {
	for(_class& i : declared_interfaces()) {
		loop_action action = handler(i);
		switch (action) {
			case loop_action::stop: return;
			case loop_action::next: continue;;
		}
	}
}
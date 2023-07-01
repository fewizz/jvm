#pragma once

#include "./layout.hpp"

template<typename Derived, typename IndexType>
struct layout_view_extension {
private:
	const Derived& derived() const & { return *(const Derived*) this; }
	      Derived& derived()       & { return *(      Derived*) this; }

	/* member functions to implement in Derived:
		// layout of fields
		layout& layout_for_view();

		// beginning of allocation of size `layout().total_size()`
		uint8* data_for_layout_view();

		// view to fields
		auto fields_for_layout_view();
	*/
	const layout& layout() { return derived().layout_for_view(); }
	uint8* data() {
		return derived().data_for_layout_view();
	}

	auto fields() { return derived().fields_view_for_layout_view(); }

public:

	decltype(auto) view_ptr(IndexType index, auto&& handler) {
		layout::slot s = layout().slot_for_field_index(index);
		uint8* ptr = data() + (uint32) s.beginning();
		field& f = fields()[index];

		return f.type.view([&]<typename Type>(Type) -> decltype(auto) {
			if constexpr(
				same_as<Type, class_file::object>
			) {
				return handler((reference*) ptr);
			}
			else if constexpr(same_as<Type, class_file::d>) {
				return handler((double*) ptr);
			}
			else if constexpr(same_as<Type, class_file::j>) {
				return handler((int64*) ptr);
			}
			else if constexpr(same_as<Type, class_file::i>) {
				return handler((int32*) ptr);
			}
			else if constexpr(same_as<Type, class_file::f>) {
				return handler((float*) ptr);
			}
			else if constexpr(same_as<Type, class_file::c>) {
				return handler((uint16*) ptr);
			}
			else if constexpr(same_as<Type, class_file::s>) {
				return handler((int16*) ptr);
			}
			else if constexpr(same_as<Type, class_file::b>) {
				return handler((int8*) ptr);
			}
			else if constexpr(same_as<Type, class_file::z>) {
				return handler((bool*) ptr);
			}
			else {
				posix::abort();
			}
		});
	}

	decltype(auto) view(IndexType index, auto&& handler) {
		return view_ptr(index, [&]<typename Type>(Type* e) -> decltype(auto) {
			Type& ref = *e;
			return handler(ref);
		});
	}

	template<typename Type>
	requires
		same_as<Type, reference> ||
		same_as<Type, int64> || same_as<Type, int32> ||
		same_as<Type, double> || same_as<Type, float> ||
		same_as<Type, int16> || same_as<Type, uint16> ||
		same_as<Type, int8> || same_as<Type, bool>
	decltype(auto) view(layout::position position, auto&& handler) {
		uint8* ptr = data() + (uint32) position;
		Type& e = * (Type*) ptr;
		return handler(e);
	}

	template<typename Type>
	Type& get(layout::position position) {
		return view<Type>(position, [](Type& e) -> Type& { return e; });
	}

	template<typename Type>
	Type& get(IndexType index) {
		return get<Type>(layout().slot_for_field_index(index).beginning());
	}

	template<typename Type>
	void set(layout::position position, Type value) {
		view<Type>(position, [&](Type& e) { return e = move(value); });
	}

};
#pragma once

#include "./stack_entry.hpp"

#include <list.hpp>
#include <bit_cast.hpp>

#include <posix/memory.hpp>

static_assert(
	sizeof(int32) == 4 && sizeof(float) == 4 && sizeof(reference) == 8 &&
	sizeof(int64) == 8 && sizeof(double) == 8
);

// inefficient for now..
template<typename Type>
concept stack_primitive_element =
	same_as<Type, int32> || same_as<Type, int64> ||
	same_as<Type, float> || same_as<Type, double>;

thread_local static class stack : list<posix::memory_for_range_of<uint64>> {
	using base_type = list<posix::memory_for_range_of<uint64>>;
	using base_type::base_type;

	posix::memory_for_range_of<uint64> reference_bits_;

	bool is_reference_at(nuint index) const {
		nuint bitmap_index = index >> 8;
		nuint bit_index = index & 63;
		return ((reference_bits_[bitmap_index].get()) >> bit_index) & 1;
	}

	template<typename Handler>
	void view_as_int32_or_reference_at(nuint index, Handler&& handler) {
		if(is_reference_at(index)) { handler(at<reference>(index)); }
		else { handler(at<int32>(index)); }
	}

	template<typename Handler>
	void pop_back_and_view_as_int32_or_reference(Handler&& handler) {
		if(is_reference_at(size() - 1)) { handler(pop_back<reference>()); }
		else { handler(pop_back<int32>()); }
	}

public:

	stack(posix::memory_for_range_of<uint64> storage) :
		base_type{ move(storage) },
		reference_bits_ {
			posix::allocate_zeroed_memory_for<uint64>(
				base_type::capacity() >> 8
			)
		}
	{}

	bool back_is_reference() const {
		return is_reference_at(size() - 1);
	}

	void pop_back() {
		if(back_is_reference()) {
			pop_back<reference>();
		}
		else {
			base_type::pop_back();
		}
	}

	void pop_back_until(nuint n) {
		while(size() > n) {
			pop_back();
		}
	}

	void pop_back(nuint n) {
		while(n > 0) {
			pop_back();
			--n;
		}
	}

	~stack() {
		pop_back_until(0);
	}

	template<stack_primitive_element Type>
	void emplace_back(Type v) {
		if constexpr(sizeof(Type) == 4) {
			base_type::emplace_back((uint64) bit_cast<uint32>(v));
		}
		else {
			base_type::emplace_back(bit_cast<uint64>(v));
			base_type::emplace_back(uint64(-1));
		}
	}
	void emplace_back(reference ref) {
		base_type::emplace_back();
		new ((char*) &base_type::back()) reference(move(ref));
		nuint index = size() - 1;
		nuint bitmap_index = index >> 8;
		nuint bit_index = index & 63;
		reference_bits_[bitmap_index].get() |= (1ull << bit_index);
	}

	template<stack_primitive_element Type>
	Type& at(nuint index) {
		return * (Type*) &base_type::operator[](index);
	}
	template<same_as<reference>>
	reference& at(nuint i) {
		return *(reference*) &base_type::operator[](i);
	}

	template<stack_primitive_element Type>
	Type& at(nuint index, Type v) {
		return at<Type>(index) = v;
	}
	reference& at(nuint i, reference ref) {
		return at<reference>(i) = move(ref);
	}

	template<stack_primitive_element Type> requires(sizeof(Type) == 4)
	Type& back() {
		return at<Type>(size() - 1);
	}
	template<stack_primitive_element Type> requires(sizeof(Type) == 8)
	Type& back() {
		return at<Type>(size() - 2);
	}
	template<same_as<reference>>
	reference& back() { return at<reference>(size() - 1); }

	template<stack_primitive_element Type>
	Type pop_back() {
		Type v = back<Type>();
		base_type::pop_back(sizeof(Type) == 4? 1 : 2);
		return v;
	}
	template<same_as<reference>>
	reference pop_back() {
		reference& ref_at_back = back<reference>();
		reference ref_moved = move(ref_at_back);
		ref_at_back.~reference();

		nuint index = size() - 1;
		nuint bitmap_index = index >> 8;
		nuint bit_index = index & 63;
		reference_bits_[bitmap_index].get() &= ~(1 << bit_index);

		base_type::pop_back();
		return ref_moved;
	}

	void dup_cat_1() {
		view_as_int32_or_reference_at(size() - 1, [&](auto& value) {
			emplace_back(value);
		});
	}

	void dup2() {
		view_as_int32_or_reference_at(size() - 2, [&](auto& value2) {
			view_as_int32_or_reference_at(size() - 1, [&](auto& value1) {
				emplace_back(value2);
				emplace_back(value1);
			});
		});
	}

	void swap_cat_1() {
		pop_back_and_view_as_int32_or_reference([&](auto value1) {
			pop_back_and_view_as_int32_or_reference([&](auto value2) {
				emplace_back(move(value1));
				emplace_back(move(value2));
			});
		});
	}

	void dup_x1_cat_1() {
		pop_back_and_view_as_int32_or_reference([&](auto value1) {
			pop_back_and_view_as_int32_or_reference([&](auto value2) {
				emplace_back(value1);
				emplace_back(move(value2));
				emplace_back(move(value1));
			});
		});
	}

	using base_type::size;

} stack{ posix::allocate_memory_for<uint64>(4096) };

struct frame : span<stack_entry> {
	using base_type = span<stack_entry>;
	using base_type::base_type;
};
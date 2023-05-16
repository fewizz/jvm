#pragma once

#include "decl/reference.hpp"

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
	same_as<Type, int32> || same_as<Type, int64>  ||
	same_as<Type, float> || same_as<Type, double> ||
	same_as<Type, int16> || same_as<Type, uint16> ||
	same_as<Type, uint8> || same_as<Type, bool>;

thread_local static class stack : list<posix::memory_for_range_of<uint64>> {
	using base_type = list<posix::memory_for_range_of<uint64>>;
	using base_type::base_type;

	posix::memory_for_range_of<uint64> reference_bits_;

	bool is_reference_at(nuint index) const {
		nuint bitmap_index = index / 64;
		nuint bit_index = index % 64;
		return ((reference_bits_[bitmap_index].get()) >> bit_index) & 1;
	}

	template<typename Handler>
	decltype(auto) view_as_int32_or_reference_at(nuint index, Handler&& handler) {
		if(is_reference_at(index)) {
			return handler(get<reference>(index));
		}
		else {
			return handler(get<int32>(index));
		}
	}

	template<typename Handler>
	decltype(auto) pop_back_and_view_as_int32_or_reference(Handler&& handler) {
		if(is_reference_at(size() - 1)) {
			return handler(pop_back<reference>());
		}
		else {
			return handler(pop_back<int32>());
		}
	}

	void unsafely_emplace_reference_at(
		nuint index,
		object_of<jl::object>* obj_ptr
	) {
		uint64* base_ptr = & base_type::operator [] (index);
		new ((char*) base_ptr) reference(obj_ptr);
		nuint bitmap_index = index / 64;
		nuint bit_index = index % 64;
		reference_bits_[bitmap_index].get() |= (uint64(1) << bit_index);
	}

	reference destruct_reference_at(nuint index) {
		reference& ref = get<reference>(index);
		reference ref_moved = move(ref);
		ref.~reference();
		base_type::pop_back();

		nuint bitmap_index = index / 64;
		nuint bit_index = index % 64;
		reference_bits_[bitmap_index].get() &= ~(uint64(1) << bit_index);

		return ref_moved;
	}

public:

	stack(nuint size) :
		base_type{ posix::allocate_memory_for<uint64>(size) },
		reference_bits_ {
			posix::allocate_zeroed_memory_for<uint64>(
				(size / 64) + (size % 64 != 0)
			)
		}
	{}

	stack(stack&& other) = default;
	stack& operator = (stack&& other) = default;

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
		//pop_back_until(0);
	}

	void emplace_back(reference ref) {
		base_type::emplace_back();
		unsafely_emplace_reference_at(size() - 1, ref.object_ptr());
	}
	void emplace_back(object_of<jl::object>& obj) {
		base_type::emplace_back();
		unsafely_emplace_reference_at(size() - 1, &obj);
	}
	template<stack_primitive_element Type>
	requires (bytes_in<Type> == 4)
	void emplace_back(Type v) {
		base_type::emplace_back((uint64) bit_cast<uint32>(v));
	}
	template<stack_primitive_element Type>
	requires (bytes_in<Type> == 8)
	void emplace_back(Type v) {
		base_type::emplace_back(bit_cast<uint64>(v));
		base_type::emplace_back(uint64(-1));
	}
	void emplace_back(int16 v) { emplace_back((int32)(uint16)(uint32) v); }
	void emplace_back(uint16 v) { emplace_back((int32)(uint32) v); }
	void emplace_back(int8 v) { emplace_back((int32)(uint32)(uint8) v); }
	void emplace_back(bool v) { emplace_back((int32) v); }

	void emplace_at(nuint index, reference ref) {
		if(is_reference_at(index)) {
			get<reference>(index) = move(ref);
		} else {
			unsafely_emplace_reference_at(index, ref.object_ptr());
		}
	}
	void emplace_at(nuint index, object_of<jl::object>& obj) {
		if(is_reference_at(index)) {
			get<reference>(index) = obj;
		} else {
			unsafely_emplace_reference_at(index, &obj);
		}
	}

	template<stack_primitive_element Type>
	void emplace_at(nuint index, Type value) {
		if(is_reference_at(index)) {
			destruct_reference_at(index);
		}
		new (& base_type::operator [] (index)) Type(value);
	}

	template<stack_primitive_element Type>
	Type& get(nuint index) {
		return * (Type*) &base_type::operator[](index);
	}
	template<same_as<reference>>
	reference& get(nuint i) {
		return *(reference*) &base_type::operator[](i);
	}

	template<stack_primitive_element Type>
	Type& get(nuint index, Type v) {
		return get<Type>(index) = v;
	}
	reference& get(nuint i, reference ref) {
		return get<reference>(i) = move(ref);
	}

	template<stack_primitive_element Type> requires(sizeof(Type) == 4)
	Type& back() {
		return get<Type>(size() - 1);
	}
	template<stack_primitive_element Type> requires(sizeof(Type) == 8)
	Type& back() {
		return get<Type>(size() - 2);
	}
	template<same_as<reference>>
	reference& back() { return get<reference>(size() - 1); }

	template<stack_primitive_element Type>
	Type pop_back() {
		Type v = back<Type>();
		base_type::pop_back(sizeof(Type) == 4? 1 : 2);
		return v;
	}
	template<same_as<reference>>
	reference pop_back() {
		return destruct_reference_at(size() - 1);
	}
	template<same_as<int16>>
	int16 pop_back() { return (uint16)(uint32) pop_back<int32>(); }
	template<same_as<uint16>>
	uint16 pop_back() { return (uint32) pop_back<int32>(); }
	template<same_as<int8>>
	int8 pop_back() { return (uint8)(uint32) pop_back<int32>(); }
	template<same_as<bool>>
	bool pop_back() { return (bool) pop_back<int32>(); }

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

	void dup_x1() {
		pop_back_and_view_as_int32_or_reference([&](auto value1) {
			pop_back_and_view_as_int32_or_reference([&](auto value2) {
				emplace_back(value1);
				emplace_back(move(value2));
				emplace_back(move(value1));
			});
		});
	}

	void dup_x2() {
		pop_back_and_view_as_int32_or_reference([&](auto value1) {
			pop_back_and_view_as_int32_or_reference([&](auto value2) {
				pop_back_and_view_as_int32_or_reference([&](auto value3) {
					emplace_back(value1);
					emplace_back(move(value3));
					emplace_back(move(value2));
					emplace_back(move(value1));
				});
			});
		});
	}

	void dup2_x1() {
		pop_back_and_view_as_int32_or_reference([&](auto value1) {
			pop_back_and_view_as_int32_or_reference([&](auto value2) {
				pop_back_and_view_as_int32_or_reference([&](auto value3) {
					emplace_back(value2);
					emplace_back(value1);
					emplace_back(move(value3));
					emplace_back(move(value2));
					emplace_back(move(value1));
				});
			});
		});
	}

	void dup2_x2() {
		pop_back_and_view_as_int32_or_reference([&](auto value1) {
			pop_back_and_view_as_int32_or_reference([&](auto value2) {
				pop_back_and_view_as_int32_or_reference([&](auto value3) {
					pop_back_and_view_as_int32_or_reference([&](auto value4) {
						emplace_back(value2);
						emplace_back(value1);
						emplace_back(move(value4));
						emplace_back(move(value3));
						emplace_back(move(value2));
						emplace_back(move(value1));
					});
				});
			});
		});
	}

	void swap() {
		pop_back_and_view_as_int32_or_reference([&](auto value1) {
			pop_back_and_view_as_int32_or_reference([&](auto value2) {
				emplace_back(move(value1));
				emplace_back(move(value2));
			});
		});
	}

	void insert_at(nuint index, reference ref) {
		if(index > size()) {
			posix::abort();
		}
		// we're at the end
		if(index == size()) {
			return emplace_back(move(ref));
		}
		// there's something at given index
		// popping it, continuing recursively
		pop_back_and_view_as_int32_or_reference([&](auto popped) {
			insert_at(index, move(ref));
			emplace_back(move(popped));
		});
	}

	template<typename Type>
	Type pop_at(nuint index) {
		nuint stack_size = same_as<Type, reference> ? 1 : sizeof(Type) / 4;
		nuint min_index = size() - stack_size;

		if(index > min_index) {
			posix::abort();
		}
		if(index == min_index) {
			return pop_back<Type>();
		}
		return pop_back_and_view_as_int32_or_reference([&](auto popped) {
			Type result = pop_at<Type>(index);
			emplace_back(move(popped));
			return result;
		});
	}

	using base_type::size;
	using base_type::capacity;

} stack{ 512 };
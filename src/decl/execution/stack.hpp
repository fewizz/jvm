#pragma once

#include "decl/reference.hpp"

#include <list.hpp>
#include <bit_cast.hpp>

#include <posix/memory.hpp>

// inefficient for now..
template<typename Type>
concept stack_primitive_element =
	same_as<Type, int32> || same_as<Type, int64>  ||
	same_as<Type, float> || same_as<Type, double> ||
	same_as<Type, int16> || same_as<Type, uint16> ||
	same_as<Type, int8> || same_as<Type, bool>;

thread_local static class stack :
	list<posix::memory<storage_of_size_and_alignment_same_as<uint64>>>
{
	using base_type = list<
		posix::memory<storage_of_size_and_alignment_same_as<uint64>>
	>;

	using base_type::base_type;

	posix::memory<uint64> reference_bits_;

	bool is_reference_at(nuint index) const {
		nuint bitmap_index = index / 64;
		nuint bit_index = index % 64;
		uint64& bitmap = reference_bits_[bitmap_index].get();
		return (bitmap >> bit_index) & 1;
	}

	template<typename Handler>
	decltype(auto)
	view_as_int32_or_reference_at(nuint index, Handler&& handler) {
		if(is_reference_at(index)) {
			return handler(get<reference>(index));
		}
		else {
			return handler(get<int32>(index));
		}
	}

	template<typename Handler>
	decltype(auto)
	pop_back_and_view_as_int32_or_reference(Handler&& handler) {
		if(is_reference_at(size() - 1)) {
			return handler(pop_back<reference>());
		}
		else {
			return handler(pop_back<int32>());
		}
	}

	void mark_reference(nuint index) {
		nuint bitmap_index = index / 64;
		nuint bit_index = index % 64;
		uint64& bitmap = reference_bits_[bitmap_index].get();
		bitmap |= (uint64(1) << bit_index);
	}

	void destruct_reference_at(nuint index) {
		base_type::operator [] (index).destruct<reference>();

		nuint bitmap_index = index / 64;
		nuint bit_index = index % 64;
		uint64& bitmap = reference_bits_[bitmap_index].get();
		bitmap &= ~(uint64(1) << bit_index);
	}

public:

	stack(nuint size) :
		base_type {
			posix::allocate<
				storage_of_size_and_alignment<sizeof(uint64), alignof(uint64)>
			>(size)
		},
		reference_bits_ {
			posix::allocate_zeroed<uint64>(
				(size / 64) + (size % 64 != 0)
			)
		}
	{}

	stack(stack&& other) = default;
	stack& operator = (stack&& other) = default;

	~stack() {
		erase_back_until(0);
	}

	bool back_is_reference() const {
		return is_reference_at(size() - 1);
	}

	void erase_back() {
		if(back_is_reference()) {
			erase_back<reference>();
		}
		else {
			base_type::erase_back();
		}
	}

	void erase_back_until(nuint n) {
		while(size() > n) {
			erase_back();
		}
	}

	void erase_back(nuint n) {
		while(n > 0) {
			erase_back();
			--n;
		}
	}

	void emplace_back(reference ref) {
		base_type::emplace_back().construct<reference>(move(ref));
		mark_reference(size() - 1);
	}
	void emplace_back(object& obj) {
		base_type::emplace_back().construct<reference>(&obj);
		mark_reference(size() - 1);
	}
	template<stack_primitive_element Type>
	requires (bytes_in<Type> == 4)
	void emplace_back(Type v) {
		base_type::emplace_back().construct<Type>(v);
	}
	template<stack_primitive_element Type>
	requires (bytes_in<Type> == 8)
	void emplace_back(Type v) {
		base_type::emplace_back().construct<Type>(v);
		base_type::emplace_back(); // two slots
	}
	void emplace_back(int16 v)  { emplace_back((int32)(uint16)(uint32) v); }
	void emplace_back(uint16 v) { emplace_back((int32)(uint32)         v); }
	void emplace_back(int8 v)   { emplace_back((int32)(uint32)(uint8)  v); }
	void emplace_back(bool v)   { emplace_back((int32)                 v); }

	void emplace_at(nuint index, reference ref) {
		if(is_reference_at(index)) {
			get<reference>(index) = move(ref);
		} else {
			base_type::operator [] (index).construct<reference>(move(ref));
			mark_reference(index);
		}
	}
	void emplace_at(nuint index, object& obj) {
		if(is_reference_at(index)) {
			get<reference>(index) = obj;
		} else {
			base_type::operator [] (index).construct<reference>(&obj);
			mark_reference(index);
		}
	}

	template<stack_primitive_element Type>
	void emplace_at(nuint index, Type value) {
		if(is_reference_at(index)) {
			destruct_reference_at(index);
		}
		base_type::operator [] (index).construct<Type>(move(value));
	}

	template<stack_primitive_element Type>
	Type& get(nuint index) {
		return base_type::operator[](index).get<Type>();
	}
	template<same_as<reference> Type>
	reference& get(nuint index) {
		return base_type::operator[](index).get<reference>();
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
	void erase_back() {
		base_type::erase_back(sizeof(Type) == 4? 1 : 2);
	}
	template<same_as<reference>>
	void erase_back() {
		destruct_reference_at(size() - 1);
		base_type::erase_back();
	}

	template<stack_primitive_element Type>
	Type pop_back() {
		Type v = back<Type>();
		erase_back<Type>();
		return v;
	}
	template<same_as<reference>>
	reference pop_back() {
		reference ref = move(back<reference>());
		erase_back<reference>();
		return ref;
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
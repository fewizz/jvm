#pragma once

#include <integer.hpp>

#include <posix/memory.hpp>

template<typename Type, nuint BlockSize>
struct blocky_memory {

	struct iterator_t {
		span<posix::memory<Type>>& blocks_{};
		nuint index_ = 0;

		iterator_t(span<posix::memory<Type>>& blocks, nuint index) :
			blocks_{ blocks },
			index_{ index }
		{}

		iterator_t(const iterator_t& other) :
			blocks_{ other.blocks_ },
			index_{ other.index_ }
		{}

		iterator_t& operator = (const iterator_t& other) {
			blocks_ = other.blocks_;
			index_ = other.index_;
			return *this;
		}

		storage<Type>& operator * () {
			return blocks_[index_ / BlockSize][index_ % BlockSize];
		}

		storage<Type>& operator * () const {
			return blocks_[index_ / BlockSize][index_ % BlockSize];
		}

		iterator_t& operator ++ () {
			++index_;
			return *this;
		}

		iterator_t& operator -- () {
			--index_;
			return *this;
		}

		iterator_t operator ++ (int) {
			iterator_t cpy{ *this };
			++index_;
			return cpy;
		}

		bool operator == (iterator_t other) const {
			return index_ == other.index_;
		}

		nuint operator - (iterator_t other) const {
			return index_ - other.index_;
		}

		iterator_t operator - (nuint n) const {
			return { blocks_, index_ - n };
		}

		iterator_t operator + (nuint n) const {
			return { blocks_, index_ + n };
		}

		iterator_t& operator += (nuint n) {
			index_ += n;
			return *this;
		}

	};

	mutable span<posix::memory<Type>> blocks{};

	nuint size() const { return blocks.size() * BlockSize; }

	void grow() const {
		span<storage<posix::memory<Type>>> new_blocks
			= posix::allocate_raw<posix::memory<Type>>(blocks.size() + 1);
		
		blocks.for_each_indexed([&](posix::memory<Type>& m, nuint index) {
			new_blocks[index].construct(move(m));
		});

		new_blocks[new_blocks.size() - 1].construct(
			posix::allocate<Type>(BlockSize)
		);

		posix::free_raw_memory(blocks.iterator());

		blocks =
			span<posix::memory<Type>> {
				(posix::memory<Type>*) new_blocks.iterator(),
				new_blocks.size()
			};
	}

	iterator_t iterator()       { return { blocks, 0 }; }
	iterator_t iterator() const { return { blocks, 0 }; }

	iterator_t sentinel()       { return { blocks, size() }; }
	iterator_t sentinel() const { return { blocks, size() }; }

};
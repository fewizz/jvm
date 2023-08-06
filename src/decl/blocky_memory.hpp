#pragma once

#include <integer.hpp>

#include <posix/memory.hpp>

template<typename Type, nuint BlockSize>
struct blocky_memory {

	struct iterator_t {
		posix::memory<posix::memory<Type>>* blocks_;
		nuint index_ = 0;

		iterator_t(posix::memory<posix::memory<Type>>& blocks, nuint index) :
			blocks_{ &blocks },
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

		Type& operator * () {
			return (*blocks_)[index_ / BlockSize][index_ % BlockSize];
		}

		Type& operator * () const {
			return (*blocks_)[index_ / BlockSize][index_ % BlockSize];
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
			return { *blocks_, index_ - n };
		}

		iterator_t operator + (nuint n) const {
			return { *blocks_, index_ + n };
		}

		iterator_t& operator += (nuint n) {
			index_ += n;
			return *this;
		}

	};

	mutable posix::memory<posix::memory<Type>> blocks{};

	blocky_memory() = default;
	blocky_memory(blocky_memory&&) = default;

	~blocky_memory() {
		for(posix::memory<Type>& block_storage : blocks) {
			block_storage.~memory<Type>();
		}
	}

	nuint size() const { return blocks.size() * BlockSize; }

	void grow() const {
		posix::memory<posix::memory<Type>> new_blocks
			= posix::allocate<posix::memory<Type>>(blocks.size() + 1);

		blocks.for_each_indexed(
			[&](posix::memory<Type>& m, nuint index) {
				new (&new_blocks[index]) posix::memory<Type>(move(m));
			}
		);

		new (&new_blocks[new_blocks.size() - 1]) posix::memory<Type>(
			posix::allocate<Type>(BlockSize)
		);

		blocks = move(new_blocks);
	}

	iterator_t iterator()       { return { blocks, 0 }; }
	iterator_t iterator() const { return { blocks, 0 }; }

	iterator_t sentinel()       { return { blocks, size() }; }
	iterator_t sentinel() const { return { blocks, size() }; }

};
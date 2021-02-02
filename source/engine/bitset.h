#pragma once
#include "array.h"
#include <algorithm>
#include <cstring>

// dynamic bitset

class Bitset
{
public:
	AllocHandle handle;
	size_t _size{};
	size_t _capacity_bytes{};

	inline Bitset() = default;
	inline explicit Bitset(size_t size) noexcept { alloc(size, size, context_allocator()); }
	inline Bitset(size_t size, size_t capacity) noexcept { alloc(size, capacity, context_allocator()); }
	inline Bitset(size_t size, Allocator allocator) noexcept { alloc(size, size, allocator); }
	inline Bitset(size_t size, size_t capacity, Allocator allocator) noexcept { alloc(size, capacity, allocator); }
	inline Bitset(const Bitset& other) noexcept { *this = other; }
	inline Bitset(Bitset&& other) noexcept { *this = std::move(other); }

	inline Bitset& operator=(const Bitset& other) noexcept;
	inline Bitset& operator=(Bitset&& other) noexcept;

	inline void alloc(size_t size, size_t capacity, Allocator allocator) noexcept;
	inline bool empty() const { return _size == 0; }
	inline size_t size() const { return _size; }
	inline void clear() { _size = 0; }
	inline void zero();
	inline void resize(size_t new_size) noexcept;
	inline void expand(size_t min_size) noexcept;
	inline bool get(size_t idx) const;
	inline void set(size_t idx, bool val) const;
	inline void push_back(bool val);
};

static constexpr inline size_t bit_to_byte_size(size_t num_bits)
{
	return (num_bits + 7) / 8;
}

static inline uint8_t* bitset_data(const Bitset& bitset)
{
	return reinterpret_cast<uint8_t*>(bitset.handle.get());
}

inline Bitset& Bitset::operator=(const Bitset& other) noexcept
{
	asserts(false); // TODO: do not enable deep copy for now, add this asserts so we are aware
	resize(other._size);
	if (_size > 0)
	{
		auto this_data = bitset_data(*this);
		auto other_data = bitset_data(other);
		asserts(this_data && other_data);
		std::memcpy(this_data, other_data, bit_to_byte_size(_size));
	}
	return *this;
}

inline Bitset& Bitset::operator=(Bitset&& other) noexcept
{
	handle = other.handle;
	_size = other._size;
	_capacity_bytes = other._capacity_bytes;
	other.handle = {};
	other._size = other._capacity_bytes = 0;
	return *this;
}

inline void Bitset::alloc(size_t size, size_t capacity, Allocator allocator) noexcept
{
	asserts(size <= capacity);
	_capacity_bytes = bit_to_byte_size(capacity);
	_size = size;
	auto& allocators = engine().allocators;
	handle = allocators.allocate(allocator, _capacity_bytes);
	zero();
}

inline void Bitset::zero()
{
	if (_size > 0)
	{
		std::memset(bitset_data(*this), 0, bit_to_byte_size(_size));
	}
}

inline void Bitset::resize(size_t new_size) noexcept
{
	const auto old_bytes = bit_to_byte_size(_size);
	const auto new_bytes = bit_to_byte_size(new_size);
	if (new_bytes > old_bytes)
	{
		if (new_bytes > _capacity_bytes)
		{
			const auto new_capacity_bytes = std::min(new_bytes, 128ull) + new_bytes;
			engine().allocators.reallocate(handle, new_capacity_bytes);
			_capacity_bytes = new_capacity_bytes;
		}
		auto data = bitset_data(*this);
		asserts(data);
		std::memset(data + old_bytes, 0, (new_bytes - old_bytes));
	}
	_size = new_size;
}

inline void Bitset::expand(size_t min_size) noexcept
{
	if (min_size > _size)
	{
		resize(min_size);
	}
}

inline bool Bitset::get(size_t idx) const
{
	asserts(idx < _size);
	const auto byte = *(bitset_data(*this) + (idx / 8));
	return (byte & ((uint8_t)1 << (idx % 8)));
}

inline void Bitset::set(size_t idx, bool val) const
{
	asserts(idx < _size);
	if (val)
	{
		*(bitset_data(*this) + (idx / 8)) |= ((uint8_t)1 << (idx % 8));
	}
	else
	{
		*(bitset_data(*this) + (idx / 8)) &= ~((uint8_t)1 << (idx % 8));
	}
}

inline void Bitset::push_back(bool val)
{
	resize(_size + 1);
	set(_size - 1, val);
}





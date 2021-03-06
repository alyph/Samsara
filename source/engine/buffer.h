#pragma once

#include "assertion.h"
#include "id.h"
#include <vector>
#include <cstring>
#include <type_traits>

class Buffer
{
public:
	static const constexpr size_t alignment = alignof(std::max_align_t);
	static const constexpr size_t alignment_mask = (alignment - 1);

	uint8_t* data{};
	size_t _size{};
	size_t _capacity{};

	Buffer() = default;
	~Buffer();
	inline Buffer(const Buffer& other) = delete;
	inline Buffer(Buffer&& other);

	inline Buffer& operator=(const Buffer& other) = delete;
	inline Buffer& operator=(Buffer&& other);

	inline uint8_t* get(size_t ptr) const;
	inline size_t size() const { return _size; }
	inline size_t capacity() const { return _capacity; }
	inline bool empty() const { return size() == 0; }
	inline void clear() { _size = 0; }
	inline void resize(size_t new_size) { resize(new_size, 0); }
	void resize(size_t new_size, size_t min_grow_size);
	void reserve(size_t new_capacity);

	inline static constexpr bool is_aligned(size_t ptr);
	inline static constexpr size_t get_next_aligned(size_t ptr);
};

#if 0
class BufferReader
{
public:
	const std::vector<uint8_t>* buffer{};
	size_t ptr{};

	operator bool() const { return {buffer != nullptr}; }
	inline const void* peek_bytes(size_t size) const;
	// inline void read_bytes(void* bytes, size_t size);
};

class BufferWriter
{
public:
	std::vector<uint8_t>* buffer{};
	size_t ptr{};

	operator bool() const { return {buffer != nullptr}; }
	inline void write_bytes(const void* bytes, size_t size);
};
#endif

inline Buffer::Buffer(Buffer&& other)
{
	*this = std::move(other);
}

inline Buffer& Buffer::operator=(Buffer&& other)
{
	std::swap(data, other.data);
	std::swap(_size, other._size);
	std::swap(_capacity, other._capacity);
	return *this;
}

inline uint8_t* Buffer::get(size_t ptr) const
{
	// TODO: bounds check? or is it overkill?
	//asserts(ptr < size());
	return (data + ptr);
}

inline constexpr bool Buffer::is_aligned(size_t ptr)
{
	return ((ptr & alignment_mask) == 0);
}

inline constexpr size_t Buffer::get_next_aligned(size_t ptr)
{
	return ((ptr + alignment_mask) & (~alignment_mask));
}


#if 0

inline const void* BufferReader::peek_bytes(size_t size) const
{
	asserts(buffer);
	// for now just assert, later if needed switch to an if check and return null
	asserts(ptr + size <= buffer->size());

	return reinterpret_cast<const void*>(buffer->data() + ptr);
}

inline void BufferWriter::write_bytes(const void* bytes, size_t size)
{
	asserts(buffer);
	if (ptr + size > buffer->size())
	{
		buffer->resize(ptr + size);
	}
	std::memcpy(reinterpret_cast<void*>(buffer->data() + ptr), bytes, size);
}

#endif

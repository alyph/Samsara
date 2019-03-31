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

	Buffer() = default;
	~Buffer();
	inline Buffer(const Buffer& other);
	inline Buffer(Buffer&& other);

	inline Buffer& operator=(const Buffer& other);
	inline Buffer& operator=(Buffer&& other);

	inline uint8_t* get(size_t ptr) const;
	inline size_t size() const { return buffer_size; }
	inline bool empty() const { return size() == 0; }
	inline void clear() { buffer_size = 0; }
	inline void resize(size_t new_size) { resize(new_size, 0); }
	void resize(size_t new_size, size_t min_grow_size);

	inline static bool is_aligned(size_t ptr);
	inline static size_t get_next_aligned(size_t ptr);

private:
	uint8_t* data{};
	size_t buffer_size{};
	size_t capacity{};
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

inline Buffer::Buffer(const Buffer& other)
{
	*this = other;
}

inline Buffer::Buffer(Buffer&& other)
{
	*this = std::move(other);
}

inline Buffer& Buffer::operator=(const Buffer& other)
{
	resize(other.size());
	std::memcpy(data, other.data, other.size());
	return *this;
}

inline Buffer& Buffer::operator=(Buffer&& other)
{
	std::swap(data, other.data);
	std::swap(buffer_size, other.buffer_size);
	std::swap(capacity, other.capacity);
	return *this;
}

inline uint8_t* Buffer::get(size_t ptr) const
{
	return (data + ptr);
}

inline bool Buffer::is_aligned(size_t ptr)
{
	return ((ptr & alignment_mask) == 0);
}

inline size_t Buffer::get_next_aligned(size_t ptr)
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

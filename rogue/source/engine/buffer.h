#pragma once

#include <vector>
#include <cstring>

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
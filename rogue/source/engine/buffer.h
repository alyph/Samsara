#pragma once

#include "assertion.h"
#include "id.h"
#include <vector>
#include <cstring>
#include <type_traits>

class BufferStore;
// template<typename T> class Buffer;

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

struct BufferHeader
{
	Id id{};
	size_t ptr{};
	uint32_t item_size{};
	uint32_t num_items{};
	uint32_t capacity{};
};

struct BufferControlBlock
{
	BufferStore* store{};
	Id id{};
	uint32_t header_ptr{};

	bool operator==(const BufferControlBlock& other) const
	{
		return (store == other.store && id == other.id && header_ptr == other.header_ptr);
	}
};



inline const BufferHeader& access_buffer_header(const BufferControlBlock& block);

template<typename T>
inline T& access_buffer_item(const BufferControlBlock& block, size_t index);

template<typename T>
class BufferIteratorImpl
{
public:
	BufferControlBlock block;
	size_t ptr{};

	T& operator*() const
	{
		return access_buffer_item<T>(block, ptr);
		// asserts(header);
		// asserts(header->id == id);
		// asserts(ptr < header->num_items);
		// return *(reinterpret_cast<T*>(header->data) + ptr);
	}

	BufferIteratorImpl& operator++()
	{
		const BufferHeader& header = access_buffer_header(block);
		asserts(ptr < header.num_items);
		++ptr;
		return *this;
	}

	bool operator==(const BufferIteratorImpl& other) const
	{
		return (block == other.block && ptr == other.ptr);
	}
};

template<typename T>
class BufferIterator
{
public:
	BufferIteratorImpl<T> impl;
	
	T& operator*() const { return *impl; }
	BufferIterator& operator++() { ++impl; return *this; }
	bool operator==(const BufferIterator& other) const { return impl == other.impl; }
	bool operator!=(const BufferIterator& other) const { return !(impl == other.impl); }
};

template<typename T>
class ConstBufferIterator
{
public:
	BufferIteratorImpl<T> impl;
	
	const T& operator*() const { return *impl; }
	ConstBufferIterator& operator++() { ++impl; return *this; }
	bool operator==(const ConstBufferIterator& other) const { return impl == other.impl; }
	bool operator!=(const ConstBufferIterator& other) const { return !(impl == other.impl); }
};

template<typename T>
class Buffer
{
public:
	static_assert(std::is_trivially_copyable_v <T>);
	
	bool empty() const { return size() == 0; }
	size_t size() const { return block.store ? access_buffer_header(block).num_items : 0; }
	inline T& operator[](size_t idx) { return at(idx); }
	inline const T& operator[](size_t idx) const { return at(idx); }
	inline T* data() { return data_impl(); }
	inline const T* data() const { return data_impl(); }
	inline BufferIterator<T> begin();
	inline ConstBufferIterator<T> begin() const;
	inline BufferIterator<T> end();
	inline ConstBufferIterator<T> end() const;

private:
	friend class BufferStore;

	inline T& at(size_t idx) const;
	inline T* data_impl() const;
	BufferControlBlock block;
	// T* data{};
};

class BufferStore
{
public:
	BufferStore();

	template<typename T>
	inline Buffer<T> allocate(size_t size);

	template<typename T>
	inline Buffer<T> allocate_reserved(size_t size, size_t capacity);

	BufferHeader* allocate_data(uint32_t item_size, size_t size, size_t capacity);

	std::vector<BufferHeader> table;
	std::vector<uint8_t> data;

private:
	size_t free_header_index{};
	size_t alloc_ptr{};
	Id last_alloc_id{};
};




// Buffer<T>

template<typename T> 
inline BufferIterator<T> Buffer<T>::begin()
{
	return { { block, 0 } };
}

template<typename T> 
inline ConstBufferIterator<T> Buffer<T>::begin() const
{
	return { { block, 0 } };
}

template<typename T> 
inline BufferIterator<T> Buffer<T>::end()
{
	return { { block, size() } };
}

template<typename T> 
inline ConstBufferIterator<T> Buffer<T>::end() const
{
	return { { block, size() } };
}

template<typename T>
inline T& Buffer<T>::at(size_t idx) const
{
	return access_buffer_item<T>(block, idx);
}

template<typename T>
inline T* Buffer<T>::data_impl() const
{
	if (size() > 0)
	{
		const BufferHeader& header = access_buffer_header(block);
		return reinterpret_cast<T*>(block.store->data.data() + header.ptr);
	}
	return nullptr;
}



// BufferStore

template<typename T>
inline Buffer<T> BufferStore::allocate(size_t size)
{
	return allocate_reserved<T>(size, size);
}

template<typename T>
inline Buffer<T> BufferStore::allocate_reserved(size_t size, size_t capacity)
{
	static_assert(std::is_trivially_copyable_v<T>);
	auto header = allocate_data(sizeof(T), size, capacity);
	Buffer<T> buffer;
	buffer.block.store = this;
	buffer.block.id = header->id;
	buffer.block.header_ptr = static_cast<decltype(BufferControlBlock::header_ptr)>(header - table.data());
	return buffer;
}

inline const BufferHeader& access_buffer_header(const BufferControlBlock& block)
{
	asserts(block.store);
	const BufferHeader& header = block.store->table[block.header_ptr];
	asserts(header.id == block.id);
	return header;
}

template<typename T>
inline T& access_buffer_item(const BufferControlBlock& block, size_t index)
{
	const BufferHeader& header = access_buffer_header(block);
	asserts(index < header.num_items);
	return *(reinterpret_cast<T*>(block.store->data.data() + header.ptr) + index);
}


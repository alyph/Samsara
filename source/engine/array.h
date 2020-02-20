#pragma once

#include "assertion.h"
#include "allocation.h"
#include "engine.h"

struct SimpleArrayHandle
{
	AllocHandle alloc_handle{};
	AllocId original_alloc_id{};

	bool operator==(const SimpleArrayHandle& other) const
	{
		return alloc_handle.header == other.alloc_handle.header && 
			original_alloc_id == other.original_alloc_id;
	}
	bool operator !=(const SimpleArrayHandle& other) const { return !(*this == other); }
};


template<typename T>
inline T& access_simple_array_item(const SimpleArrayHandle& handle, size_t index);

template<typename T>
class SimpleArrayIteratorImpl
{
public:
	SimpleArrayHandle handle;
	size_t ptr{};
	size_t size{};

	T& operator*() const
	{
		return access_simple_array_item<T>(handle, ptr);
	}

	SimpleArrayIteratorImpl& operator++()
	{
		asserts(ptr < size);
		++ptr;
		return *this;
	}

	bool operator==(const SimpleArrayIteratorImpl& other) const
	{
		// must point to the same array
		if (handle != other.handle)
			return false;

		// all end() are the same
		const bool this_is_end = (ptr == size);
		const bool other_is_end = (other.ptr == other.size);
		if (this_is_end && other_is_end)
		{
			return true;
		}
		else if (!this_is_end && !other_is_end)
		{
			// ignore size here
			return (ptr == other.ptr);
		}
		else
		{
			return false; // 1 end, 1 not
		}
	}
};

template<typename T>
class SimpleArrayIterator
{
public:
	SimpleArrayIteratorImpl<T> impl;
	
	T& operator*() const { return *impl; }
	SimpleArrayIterator& operator++() { ++impl; return *this; }
	bool operator==(const SimpleArrayIterator& other) const { return impl == other.impl; }
	bool operator!=(const SimpleArrayIterator& other) const { return !(impl == other.impl); }
};

template<typename T>
class ConstSimpleArrayIterator
{
public:
	SimpleArrayIteratorImpl<T> impl;
	
	const T& operator*() const { return *impl; }
	ConstSimpleArrayIterator& operator++() { ++impl; return *this; }
	bool operator==(const ConstSimpleArrayIterator& other) const { return impl == other.impl; }
	bool operator!=(const ConstSimpleArrayIterator& other) const { return !(impl == other.impl); }
};


struct SimpleArrayHeader
{
	uint32_t size{};
	uint32_t capacity{};
};

// make sure it s pow of 2, so it will be at alignment boundary, and then see below alignment check
// basically we can't have the header affect the alignment of the array items
static_assert((sizeof(SimpleArrayHeader) & (sizeof(SimpleArrayHeader) - 1)) == 0);

// custom allocated
// trivially copyable and sharable
// manual allocation and deallocation
// trivial elements (no construction or destruction)
// TODO: the requirements of sharable causes 2 problem
// 1. size must be stored in the allocated memory, slower than inlined, 
// 2. out of capacity resizing requires to bump allocation counter for notifying
//    shared instances, which causes more refreshing on other allocated handles
// so maybe worth making a non-sharable array first, then figure out a different
// way to handle sharing, e.g. allocating a separate shared control block
// just like how shared pointer is implemented
template<typename T>
class SimpleArray
{
public:
	static_assert(std::is_trivially_copyable_v <T>);
	static_assert(alignof(T) <= sizeof(SimpleArrayHeader));

	SimpleArrayHandle handle;

	inline void alloc_stored(size_t size, size_t capacity);
	inline void alloc_temp(size_t size, size_t capacity);
	inline bool empty() const { return size() == 0; }
	inline size_t size() const;
	inline void zero();
	inline T& operator[](size_t idx) { return at(idx); }
	inline const T& operator[](size_t idx) const { return at(idx); }
	inline T* data() { return data_impl(); }
	inline const T* data() const { return data_impl(); }
	inline SimpleArrayIterator<T> begin();
	inline ConstSimpleArrayIterator<T> begin() const;
	inline SimpleArrayIterator<T> end();
	inline ConstSimpleArrayIterator<T> end() const;
	inline T& back() { return back_impl(); } // TODO: add a version that takes a num to return n to the last item
	inline const T& back() const { return back_impl(); }
	inline void resize(size_t new_size);
	inline void push_back(const T& value);
	inline void pop_back();
	inline void insert(size_t pos, const T& value);
	inline void insert_zeroes(size_t pos, size_t count);

private:
	inline T& at(size_t idx) const;
	inline T* data_impl() const;
	inline T& back_impl() const;
	inline SimpleArrayHeader* access_header() const;
	inline void ensure_capacity(size_t size);
};

template<typename T>
constexpr size_t total_simple_array_buffer_size(size_t num_items)
{
	return sizeof(SimpleArrayHeader) + sizeof(T) * num_items;
}

template<typename T>
SimpleArray<T> alloc_simple_array(size_t size, size_t capacity, bool temp=false)
{
	asserts(size <= capacity && capacity > 0);
	const size_t num_bytes = total_simple_array_buffer_size<T>(capacity);
	auto& allocators = engine().allocators;
	const auto allocator = (temp ? allocators.current_temp_allocator : Allocator::persistent);
	auto handle = allocators.allocate(allocator, num_bytes);
	auto header = reinterpret_cast<SimpleArrayHeader*>(handle.ptr);
	header->size = static_cast<decltype(header->size)>(size);
	header->capacity = static_cast<decltype(header->capacity)>(capacity);
	return { { handle, handle.alloc_id } };
}

template<typename T>
SimpleArray<T> alloc_simple_array(size_t size, bool temp=false)
{
	return alloc_simple_array<T>(size, size, temp);
}

template<typename T>
inline T& access_simple_array_item(const SimpleArrayHandle& handle, size_t index)
{
	// no bounds check since anything calling this should already do bounds check if necessary
	auto alloc_data = reinterpret_cast<uint8_t*>(handle.alloc_handle.get(engine().allocators));
	auto data = reinterpret_cast<T*>(alloc_data + sizeof(SimpleArrayHeader));
	return *(data + index);
}

// SimpleArray<T>

template<typename T>
inline void SimpleArray<T>::alloc_stored(size_t size, size_t capacity)
{
	*this = alloc_simple_array<T>(size, capacity, false);
}
template<typename T>
inline void SimpleArray<T>::alloc_temp(size_t size, size_t capacity)
{
	*this = alloc_simple_array<T>(size, capacity, true);
}

template<typename T>
inline size_t SimpleArray<T>::size() const
{
	const auto header = access_header();
	return header ? header->size : 0;
}

template<typename T>
inline void SimpleArray<T>::zero()
{
	// TODO: safe null pointer when size is also 0?
	memset(data(), 0, sizeof(T) * size());
}

template<typename T>
inline SimpleArrayIterator<T> SimpleArray<T>::begin()
{
	return { { handle, 0, size() } };
}

template<typename T>
inline ConstSimpleArrayIterator<T> SimpleArray<T>::begin() const
{
	return { { handle, 0, size() } };
}

template<typename T>
inline SimpleArrayIterator<T> SimpleArray<T>::end()
{
	return { { handle, 0, 0 } };
}

template<typename T>
inline ConstSimpleArrayIterator<T> SimpleArray<T>::end() const
{
	return { { handle, 0, 0 } };
}

template<typename T>
constexpr T* data_adddress_from_header(SimpleArrayHeader* header)
{
	return reinterpret_cast<T*>(header + 1); // the bytes after header is the data
}

template<typename T>
inline void SimpleArray<T>::resize(size_t new_size)
{
	ensure_capacity(new_size);
	auto header = access_header();
	header->size = static_cast<decltype(header->size)>(new_size);
}

template<typename T>
inline void SimpleArray<T>::push_back(const T& value)
{
	auto header = access_header();
	asserts(header); // cannot call push_back before allocate an array
	if (header->size >= header->capacity)
	{
		ensure_capacity(header->size + 1);
		header = access_header();
	}
	*(data_adddress_from_header<T>(header) + header->size) = value;
	header->size++;
}

template<typename T>
inline void SimpleArray<T>::pop_back()
{
	auto header = access_header();
	asserts(header && header->size > 0); // cannot call pop_back if array is empty
	header->size--;
}

template<typename T>
inline void SimpleArray<T>::insert(size_t pos, const T& value)
{
	const auto s = size();
	asserts(pos <= s);
	resize(s + 1);
	auto d = data();
	memmove(d + pos + 1, d + pos, (s - pos) * sizeof(T));
	*(d + pos) = value;
}

template<typename T>
inline void SimpleArray<T>::insert_zeroes(size_t pos, size_t count)
{
	const auto s = size();
	asserts(pos <= s);
	resize(s + count);
	auto d = data();
	memmove(d + pos + count, d + pos, (s - pos) * sizeof(T));
	memset(d + pos, 0, count * sizeof(T));
}

template<typename T>
inline T& SimpleArray<T>::at(size_t idx) const
{
	asserts(idx < size());
	return access_simple_array_item<T>(handle, idx);
}

template<typename T>
inline T* SimpleArray<T>::data_impl() const
{
	const auto header = access_header();
	if (header && header->size > 0)
	{
		return data_adddress_from_header<T>(header);
	}
	return nullptr;
}

template<typename T>
inline T& SimpleArray<T>::back_impl() const
{
	const auto header = access_header();
	asserts(header && header->size > 0);
	return *(data_adddress_from_header<T>(header) + header->size - 1);
}

template<typename T>
inline SimpleArrayHeader* SimpleArray<T>::access_header() const
{
	return reinterpret_cast<SimpleArrayHeader*>(handle.alloc_handle.get(engine().allocators));
}

template<typename T>
inline void SimpleArray<T>::ensure_capacity(size_t size)
{
	auto header = access_header();
	asserts(header);
	if (size > header->capacity)
	{
		size_t new_capacity = (size * 2);
		engine().allocators.reallocate(handle.alloc_handle, new_capacity);
		header = access_header();
		header->capacity = (decltype(header->capacity))new_capacity;
	}
}


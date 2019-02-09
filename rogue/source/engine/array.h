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


// custom allocated
// trivially copyable and sharable
// manual allocation and deallocation
// trivial elements (no construction or destruction)
template<typename T>
class SimpleArray
{
public:
	static_assert(std::is_trivially_copyable_v <T>);

	SimpleArrayHandle handle;

	bool empty() const { return size() == 0; }
	inline size_t size() const;
	inline T& operator[](size_t idx) { return at(idx); }
	inline const T& operator[](size_t idx) const { return at(idx); }
	inline T* data() { return data_impl(); }
	inline const T* data() const { return data_impl(); }
	inline SimpleArrayIterator<T> begin();
	inline ConstSimpleArrayIterator<T> begin() const;
	inline SimpleArrayIterator<T> end();
	inline ConstSimpleArrayIterator<T> end() const;

private:
	inline T& at(size_t idx) const;
	inline T* data_impl() const;
};

template<typename T>
SimpleArray<T> alloc_simple_array(size_t size, size_t capacity, bool temp=false)
{
	asserts(size <= capacity);
	const size_t num_bytes = sizeof(SimpleArrayHeader) + sizeof(T) * capacity;
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
inline size_t SimpleArray<T>::size() const
{
	const auto header = reinterpret_cast<SimpleArrayHeader*>(handle.alloc_handle.get(engine().allocators));
	return header ? header->size : 0;
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
inline T& SimpleArray<T>::at(size_t idx) const
{
	asserts(idx < size());
	return access_simple_array_item<T>(handle, idx);
}

template<typename T>
inline T* SimpleArray<T>::data_impl() const
{
	const auto header = reinterpret_cast<SimpleArrayHeader*>(handle.alloc_handle.get(engine().allocators));
	if (header && header->size > 0)
	{
		return reinterpret_cast<T*>(header + 1); // the bytes after header is the data
	}
	return nullptr;
}


#pragma once

#include "assertion.h"
#include "allocation.h"
#include "reflection.h"
#include "engine.h"

// Array
// - Array is a dynamically expandable continguous ist of same typed elements
// - default initialized Array is empty and not writable
//    - it must be initailized with a designated allocator and optionally a capacity
// - copy constructor and assignment performs deep of all items into target's allocator's allocated memory
//    - if the target array is default initialized and no allocator assigned, it will be assigned to temp allocator
// - move constructor and assignment clones entire content to the target array and initialize the source array back to empty
// - Array does not deconstruct its elements nor deallocate its memory upon destruction (as allocator does not require you to deallocate memory)
// - Array can still be trivially copied (copy the pointer) in some occassions (like when used with attributes)
//    - but users should take precaution when:
//       1. giving out the trivial copy (knowing you should not modify the content during the known life time of such copy, e.g. for attribute it is until the next frame)
//       2. storing a trivial copy (knowning such array may be modified and become stale or even invalid since the original array may change in the foreseeable future)

#if 0
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
#endif

template<typename T>
class ArrayIteratorImpl
{
public:
	T* data{};
	size_t ptr{};
	size_t size{};

	T& operator*() const
	{
		asserts(ptr < size);
		return *(data + ptr);
	}

	ArrayIteratorImpl& operator++()
	{
		asserts(ptr < size);
		++ptr;
		return *this;
	}

	bool operator==(const ArrayIteratorImpl& other) const
	{
		// must point to the same array
		if (data != other.data)
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
class ArrayIterator
{
public:
	ArrayIteratorImpl<T> impl;
	
	T& operator*() const { return *impl; }
	ArrayIterator& operator++() { ++impl; return *this; }
	bool operator==(const ArrayIterator& other) const { return impl == other.impl; }
	bool operator!=(const ArrayIterator& other) const { return !(impl == other.impl); }
};

template<typename T>
class ConstArrayIterator
{
public:
	ArrayIteratorImpl<T> impl;
	
	const T& operator*() const { return *impl; }
	ConstArrayIterator& operator++() { ++impl; return *this; }
	bool operator==(const ConstArrayIterator& other) const { return impl == other.impl; }
	bool operator!=(const ConstArrayIterator& other) const { return !(impl == other.impl); }
};


#if 0
struct SimpleArrayHeader
{
	uint32_t size{};
	uint32_t capacity{};
};

// make sure it s pow of 2, so it will be at alignment boundary, and then see below alignment check
// basically we can't have the header affect the alignment of the array items
static_assert((sizeof(SimpleArrayHeader) & (sizeof(SimpleArrayHeader) - 1)) == 0);
#endif

template<typename T>
class ArrayView
{
public:
	static_assert(std::is_trivial_v<T> || std::is_trivially_destructible_v<T>);
	static_assert(alignof(T) <= alignof(std::max_align_t));

	AllocHandle handle;
	size_t _first{};
	size_t _size{};

	inline bool empty() const { return size() == 0; }
	inline size_t size() const { return _size; }
	inline T& operator[](size_t idx) { return at(idx); }
	inline const T& operator[](size_t idx) const { return at(idx); }
	inline T* data() { return empty() ? nullptr : data_ptr(); }
	inline const T* data() const { return empty() ? nullptr : data_ptr(); }
	inline ArrayIterator<T> begin() { return {{ data_ptr(), 0, size() }}; }
	inline ConstArrayIterator<T> begin() const { return {{ data_ptr(), 0, size() }}; }
	inline ArrayIterator<T> end() { return {{ data_ptr(), size(), size() }}; }
	inline ConstArrayIterator<T> end() const { return {{ data_ptr(), size(), size() }}; }
	inline T& back() { return back_impl(); } // TODO: add a version that takes a num to return n to the last item
	inline const T& back() const { return back_impl(); }

private:
	inline T& at(size_t idx) const { asserts(idx < size()); return *(data_ptr() + idx); }
	inline T* data_ptr() const { return (reinterpret_cast<T*>(handle.get()) + _first); }
	inline T& back_impl() const { asserts(!empty()); return *(data_ptr() + _size - 1); }
};

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
class ArrayBase
{
public:
	static_assert(std::is_trivial_v<T> || std::is_trivially_destructible_v<T>);
	static_assert(alignof(T) <= alignof(std::max_align_t));
	using ElemType = T;

	AllocHandle handle;
	size_t _size{};
	size_t _capacity{};

	inline bool empty() const { return size() == 0; }
	inline size_t size() const { return _size; }
	inline T& operator[](size_t idx) { return at(idx); }
	inline const T& operator[](size_t idx) const { return at(idx); }
	inline T* data() { return empty() ? nullptr : data_ptr(); }
	inline const T* data() const { return empty() ? nullptr : data_ptr(); }
	inline ArrayIterator<T> begin();
	inline ConstArrayIterator<T> begin() const;
	inline ArrayIterator<T> end();
	inline ConstArrayIterator<T> end() const;
	inline T& back() { return back_impl(); } // TODO: add a version that takes a num to return n to the last item
	inline const T& back() const { return back_impl(); }
	inline void clear() { _size = 0; }
	inline void resize(size_t new_size);
	inline void expand(size_t min_size);
	inline void push_back(const T& value); // TODO: move version
	inline void pop_back();
	inline void insert(size_t pos, const T& value);
	inline void insert_defaults(size_t pos, size_t count);
	inline ArrayView<T> view() { return { handle, 0, size() }; }
	static constexpr bool is_trivial() { return (std::is_trivial_v<T> || (std::is_trivially_constructible_v<T> && std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>)); }

protected:
	inline void alloc_impl(size_t size, size_t capacity, Allocator allocator) noexcept;
	inline void alloc_impl(Allocator allocator, std::initializer_list<T> il) noexcept;
	inline void init_data(size_t first, size_t count) noexcept;
	inline T& at(size_t idx) const;
	inline T* data_ptr() const { return reinterpret_cast<T*>(handle.get()); }
	inline T& back_impl() const;
	inline void ensure_capacity(size_t size);
};



template<typename T>
class Array: public ArrayBase<T>
{
public:
	inline Array() = default;
	inline Array(size_t size, size_t capacity) noexcept { this->alloc_impl(size, capacity, perm_allocator()); } // TODO: should not use default allocator, pass in a boolean to decide whether to use perm or temp allocator
	inline Array(size_t size, Allocator allocator) noexcept { this->alloc_impl(size, size, allocator); }
	inline Array(size_t size, size_t capacity, Allocator allocator) noexcept { this->alloc_impl(size, capacity, allocator); }
	// inline Array(const Array<T>& other) noexcept { *this = other; } // TODO: don't implement until we really need this
	inline Array(Array<T>&& other) noexcept { *this = std::move(other); }
	// inline Array& operator=(const Array<T>& other) noexcept; // TODO: don't implement until we really need this
	inline Array& operator=(Array<T>&& other) noexcept; // TODO: not implemented yet, implement it if we actually need to deep copy the array

	inline void alloc(size_t size, size_t capacity, Allocator allocator) { this->alloc_impl(size, capacity, allocator); }
	inline void alloc(Allocator allocator, std::initializer_list<T> il) { this->alloc_impl(allocator, il); }
	inline void alloc_perm(size_t size, size_t capacity) { this->alloc_impl(size, capacity, perm_allocator()); }
	inline void alloc_perm(std::initializer_list<T> il) { this->alloc_impl(perm_allocator(), il); }
	inline void alloc_temp(size_t size, size_t capacity) { this->alloc_impl(size, capacity, temp_allocator()); }
	inline void alloc_temp(std::initializer_list<T> il) { this->alloc_impl(temp_allocator(), il); }
};

template<typename T>
inline Array<T> make_temp_array(size_t size, size_t capacity);

// TODO: consider renaming this
// essentially this is a trivially copyable array that does not deep copy each element when copied
template<typename T>
class ArrayTemp: public ArrayBase<T>
{
public:
	inline ArrayTemp() = default;
	inline explicit ArrayTemp(size_t size) noexcept { alloc(size, size); }
	inline ArrayTemp(size_t size, size_t capacity) noexcept { alloc(size, capacity); }
	// TODO: maybe rename these to alloc_temp just to make it clear and explicit
	inline void alloc(size_t size) { alloc(size, size); }
	inline void alloc(size_t size, size_t capacity) { this->alloc_impl(size, capacity, temp_allocator()); }
	inline void alloc(std::initializer_list<T> il) { this->alloc_impl(temp_allocator(), il); }
	inline operator const Array<T>&() const { return *reinterpret_cast<const Array<T>*>(this); }
	inline operator Array<T>&() { return *reinterpret_cast<Array<T>*>(this); }
};



#if 0
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
#endif

// Array<T>

template<typename T>
constexpr size_t total_array_buffer_size(size_t num_items)
{
	return sizeof(T) * num_items;
}

template<typename T>
inline void ArrayBase<T>::alloc_impl(size_t size, size_t capacity, Allocator allocator) noexcept
{
	asserts(size <= capacity);
	const size_t num_bytes = total_array_buffer_size<T>(capacity);
	auto& allocators = engine().allocators;
	handle = allocators.allocate(allocator, num_bytes);
	_capacity = capacity;
	_size = size;
	init_data(0, size);
}

template<typename T>
inline void ArrayBase<T>::alloc_impl(Allocator allocator, std::initializer_list<T> il) noexcept
{
	const size_t num = il.size();
	alloc_impl(il.size(), num, allocator);
	if constexpr (is_trivial())
	{
		memcpy(data_ptr(), il.begin(), num * sizeof(T));
	}
	else
	{
		// TODO: use copy constructor instead of default construction then assignment
		auto ptr = data_ptr();
		auto begin = il.begin();
		for (size_t i = 0; i < num; i++)
		{
			*(ptr + i) = *(begin + i);
		}
	}
}

#if 0
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
#endif

template<typename T>
inline ArrayIterator<T> ArrayBase<T>::begin()
{
	return { { data_ptr(), 0, size() } };
}

template<typename T>
inline ConstArrayIterator<T> ArrayBase<T>::begin() const
{
	return { { data_ptr(), 0, size() } };
}

template<typename T>
inline ArrayIterator<T> ArrayBase<T>::end()
{
	return { { data_ptr(), size(), size() } };
}

template<typename T>
inline ConstArrayIterator<T> ArrayBase<T>::end() const
{
	return { { data_ptr(), size(), size() } };
}

#if 0
template<typename T>
constexpr T* data_adddress_from_header(SimpleArrayHeader* header)
{
	return reinterpret_cast<T*>(header + 1); // the bytes after header is the data
}
#endif

template<typename T>
inline void ArrayBase<T>::resize(size_t new_size)
{
	ensure_capacity(new_size);
	const size_t old_size = _size;
	_size = new_size;
	if (old_size < new_size)
	{
		init_data(old_size, (new_size - old_size));
	}
}

template<typename T>
inline void ArrayBase<T>::expand(size_t min_size)
{
	if (_size < min_size)
	{
		resize(min_size);
	}
}

template<typename T>
inline void ArrayBase<T>::push_back(const T& value)
{
	if (_size >= _capacity)
	{
		ensure_capacity(_size + 1);
	}
	// TODO: perform copy construction if the type is not trivial
	*(data_ptr() + _size) = value;
	_size++;
}

template<typename T>
inline void ArrayBase<T>::pop_back()
{
	asserts(_size > 0); // cannot call pop_back if array is empty
	_size--;
}

template<typename T>
inline void ArrayBase<T>::insert(size_t pos, const T& value)
{
	const auto s = size();
	asserts(pos <= s);
	ensure_capacity(s + 1);
	_size = (s + 1);
	auto d = data_ptr();
	memmove(d + pos + 1, d + pos, (s - pos) * sizeof(T));
	*(d + pos) = value;
}

template<typename T>
inline void ArrayBase<T>::insert_defaults(size_t pos, size_t count)
{
	const auto s = size();
	asserts(pos <= s);
	ensure_capacity(s + count);
	_size = (s + count);
	auto d = data_ptr();
	memmove(d + pos + count, d + pos, (s - pos) * sizeof(T));
	init_data(pos, count);
}

template<typename T>
inline T& ArrayBase<T>::at(size_t idx) const
{
	asserts(idx < size());
	return *(data_ptr() + idx);
}

#if 0
template<typename T>
inline T* ArrayBase<T>::data_impl() const
{
	// const auto header = access_header();
	// if (header && header->size > 0)
	// {
	// 	return data_adddress_from_header<T>(header);
	// }
	// return nullptr;
	return empty() ? nullptr : (reinterpret_cast<T*>(handle.get()));
}
#endif

template<typename T>
inline T& ArrayBase<T>::back_impl() const
{
	asserts(!empty());
	return *(data_ptr() + size() - 1);
}

template<typename T>
inline void ArrayBase<T>::ensure_capacity(size_t size)
{
	if (size > _capacity)
	{
		// TODO: we may be dangerously increasing the size too much later on
		// for large data storage it might be too much
		size_t new_capacity = (size * 2);
		engine().allocators.reallocate(handle, total_array_buffer_size<T>(new_capacity));
		_capacity = new_capacity;
	}
}

template<typename T>
inline void ArrayBase<T>::init_data(size_t first, size_t count) noexcept
{
	T* d = data_ptr();
	// TODO: user provided constructor is ignored
	// becauase there's no way to distinguish between user defined constructor
	// and default member initalizer
	// maybe we should now stop using 0 initalization (e.g. foo{};) on memeber init 
	// and just rely on zero initalization in code everywhere
	if constexpr (is_trivial())
	{
		memset(d + first, 0, sizeof(T) * count);
	}
	else
	{
		const size_t end = (first + count);
		for (size_t i = first; i < end; i++)
		{
			new(d + i) T;
		}
	}
}

// namespace attribute_serialization
// {
// 	template<typename T>
// 	struct can_be_trivially_stored<Array<T>>
// 	{
// 		static const constexpr bool value = true;
// 	};
// }

template<typename T>
inline Array<T>& Array<T>::operator=(Array<T>&& other) noexcept
{
	this->handle = other.handle;
	this->_size = other._size;
	this->_capacity = other._capacity;
	other.handle = {};
	other._size = 0;
	other._capacity = 0;
	return *this;
}

template<typename T>
inline Array<T> make_temp_array(size_t size, size_t capacity) 
{ 
	Array<T> array; 
	array.alloc_temp(size, capacity); 
	return array; 
}


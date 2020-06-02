#pragma once

#include "array.h"

// collection of elements with unique id
// TODO: should we rename this to something like Archive or Table?



template<typename T>
class CollectionIteratorBase
{
public:
	T* data{};
	size_t ptr{};
	size_t size{};

protected:
	T& get_impl() const
	{
		asserts(ptr < size);
		return *(data + ptr);
	}
	void next_impl()
	{
		asserts(ptr < size);
		while (++ptr < size)
		{
			if (data[ptr].id) break;
		}
	}
	bool equal_impl(const CollectionIteratorBase& other) const
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
class CollectionIterator: public CollectionIteratorBase<T>
{
public:
	T& operator*() const { return get_impl(); }
	CollectionIterator& operator++() { next_impl(); return *this; }
	bool operator==(const CollectionIterator& other) const { return equal_impl(other); }
	bool operator!=(const CollectionIterator& other) const { return !equal_impl(other); }
};

template<typename T>
class ConstCollectionIterator: public CollectionIteratorBase<T>
{
public:
	const T& operator*() const { return get_impl(); }
	ConstCollectionIterator& operator++() { next_impl(); return *this; }
	bool operator==(const ConstCollectionIterator& other) const { return equal_impl(other); }
	bool operator!=(const ConstCollectionIterator& other) const { return !equal_impl(other); }
};

template<typename T>
class Collection
{
public:
	Array<T> array;

	inline void alloc(size_t capactiy, Allocator allocator) { array.alloc(0, capactiy, allocator); }
	inline T& emplace(); // TODO: version with parameters
	inline void erase(Id id);
	inline T& get(Id id);
	inline const T& get(Id id) const;
	inline CollectionIterator<T> begin() { return { array.data(), 0, array.size() }; }
	inline ConstCollectionIterator<T> begin() const { return { array.data(), 0, array.size() }; }
	inline CollectionIterator<T> end() { return { array.data(), array.size(), array.size() }; }
	inline ConstCollectionIterator<T> end() const { return { array.data(), array.size(), array.size() }; }
};

template<typename T>
inline T& Collection<T>::emplace()
{
	for (size_t i = 0; i < array.size(); i++)
	{
		if (!array[i].id)
		{
			T& item = array[i];
			item.id = index_to_id(i);
			return item;
		}
	}

	array.resize(array.size() + 1);
	T& new_item = array.back();
	new_item.id = index_to_id(array.size() - 1);
	return new_item;
}

template<typename T>
inline void Collection<T>::erase(Id id)
{
	const auto idx = id_to_index(id);
	asserts(idx < array.size());
	array[idx].id = null_id;
}

template<typename T>
inline T& Collection<T>::get(Id id)
{
	T& item = array[id_to_index(id)];
	asserts(item.id);
	return item;
}

template<typename T>
inline const T& Collection<T>::get(Id id) const
{
	return const_cast<Collection<T>*>(this)->get(id);
}




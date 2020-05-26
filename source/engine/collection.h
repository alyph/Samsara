#pragma once

#include "array.h"

// collection of elements with unique id
// TODO: should we rename this to something like Archive or Table?

template<typename T>
class Collection
{
public:
	Array<T> array;

	inline void alloc(size_t capactiy, Allocator allocator) { array.alloc(0, capactiy, allocator); }
	inline T& emplace(); // TODO: version with parameters
	inline void erase(Id id);
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





#pragma once

#include "array.h"
#include "serialization.h"

// collection of elements with unique id
// id 0 can be set too, useful for modeling a null object
// TODO: should we rename this to something like Archive or Table?

template<typename T>
class CollectionIterator
{
public:
	T* data{};
	size_t ptr{};
	size_t size{};

	T& operator*() const;
	CollectionIterator& operator++();
	bool operator==(const CollectionIterator& other) const;
	bool operator!=(const CollectionIterator& other) const { return !(*this == other); }
};

template<typename T>
using ConstCollectionIterator = CollectionIterator<const T>;

template<typename T>
using ConditionalConstArray = std::conditional_t<std::is_const_v<T>, const Array<std::remove_const_t<T>>, Array<T>>;

template<typename T>
inline static CollectionIterator<T> collection_make_begin_iter(ConditionalConstArray<T>& array);

struct CollectionKeyEntry
{
	String name;
	Id hash;
};

template<typename T>
class Collection
{
public:
	
	using ElemType = T;
	Array<T> array;
	Array<CollectionKeyEntry> keys;
	
	inline void alloc(size_t capactiy, Allocator allocator);
	inline bool has(Id id) const;
	inline T& get(Id id);
	inline const T& get(Id id) const;
	inline T& set(Id id, T&& elem);
	inline T& set(const String& key, T&& elem);
	inline T& set(Id id, const String& key, T&& elem);
	inline T& emplace(); // TODO: version with parameters
	inline T& emplace(const String& key);
	inline void erase(Id id);
	inline void clear();
	inline bool id_to_key(Id id, String& out_key) const;
	inline bool key_to_id(const String& key, Id& out_id) const;
	inline bool key_hash_to_id(Id hash, Id& out_id) const;
	inline CollectionIterator<T> begin() { return collection_make_begin_iter<T>(array); }
	inline ConstCollectionIterator<T> begin() const { return collection_make_begin_iter<const T>(array); }
	inline CollectionIterator<T> end() { return { array.data(), array.size(), array.size() }; }
	inline ConstCollectionIterator<T> end() const { return { array.data(), array.size(), array.size() }; }
};

template<typename T>
inline static CollectionIterator<T> collection_make_begin_iter(ConditionalConstArray<T>& array)
{
	CollectionIterator<T> begin{array.data(), 0, array.size()};
	if (array.size() > 0 && array[0].id != null_id)
	{
		++begin;
	}
	return begin;
}

template<typename T>
T& CollectionIterator<T>::operator*() const
{
	asserts(ptr < size);
	return *(data + ptr);
}

template<typename T>
CollectionIterator<T>& CollectionIterator<T>::operator++()
{
	asserts(ptr < size);
	while (++ptr < size)
	{
		if (data[ptr].id) break;
	}
	return *this;
}

template<typename T>
bool CollectionIterator<T>::operator==(const CollectionIterator& other) const
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

inline static constexpr size_t collection_id_to_index(Id id) { return id; }
inline static constexpr Id collection_index_to_id(size_t index) { return index; }

inline static void collection_set_key(Array<CollectionKeyEntry>& keys, Id id, const String& key, Id hash)
{
	asserts(!key.empty());
	asserts(hash);
	const auto index = collection_id_to_index(id);
	keys.expand(index + 1);
	keys[index].name.store(key);
	keys[index].hash = hash;
}

template<typename T>
inline void Collection<T>::alloc(size_t capactiy, Allocator allocator)
{ 
	array.alloc(1, capactiy, allocator);
	keys.alloc(0, 0, allocator);
	// an element at 0 with id set to -1 (!== 0) to indicate it's not set
	// array[0] = T{};
	array[0].id = -1;
}

template<typename T>
inline T& Collection<T>::emplace()
{
	// starting search from 1
	// 0 is reserved
	for (size_t i = 1; i < array.size(); i++)
	{
		if (!array[i].id)
		{
			T& item = array[i];
			item.id = collection_index_to_id(i);
			return item;
		}
	}

	// Always reserve for 0 id element, so minmum size will now be 2
	array.resize(std::max(array.size() + 1, 2ull));
	T& new_item = array.back();
	new_item.id = collection_index_to_id(array.size() - 1);
	return new_item;
}

template<typename T>
inline T& Collection<T>::emplace(const String& key)
{
	const auto key_hash = key.hash();
	Id existing_id;
	asserts(!key_hash_to_id(key_hash, existing_id));
	T& new_item = emplace();
	collection_set_key(keys, new_item.id, key, key_hash);
	return new_item;
}

template<typename T>
inline void Collection<T>::erase(Id id)
{
	asserts(id);
	const auto idx = collection_id_to_index(id);
	array[idx].id = (id ? null_id : -1); // mark -1 for 0, and 0 for all others
	// clear the key if exists
	if (idx < keys.size())
	{
		keys[idx].name.clear();
		keys[idx].hash = 0;
	}
}

template<typename T>
inline T& Collection<T>::set(Id id, T&& elem)
{
	const auto idx = collection_id_to_index(id);
	if (idx >= array.size())
	{
		array.resize(idx + 1);
	}
	auto& new_elem = array[idx];
	new_elem = std::move(elem);
	new_elem.id = id; // must set id after setting the element, because that will overwrite the id field
	return new_elem;
}

template<typename T>
inline T& Collection<T>::set(const String& key, T&& elem)
{
	const auto key_hash = key.hash();
	Id existing_id;
	if (key_hash_to_id(key_hash, existing_id))
	{
		return set(existing_id, std::move(elem));
	}
	else
	{
		T& new_elem = emplace();
		collection_set_key(keys, new_elem.id, key, key_hash);
		return set(new_elem.id, std::move(elem));
	}
}

template<typename T>
inline T& Collection<T>::set(Id id, const String& key, T&& elem)
{
	const auto key_hash = key.hash();
	Id existing_id;
	asserts(!key_hash_to_id(key_hash, existing_id)); // Maybe we don't assert if the hash is the same for the given id, also match the key string, will be noop
	collection_set_key(keys, id, key, key_hash);
	return set(id, std::move(elem));
}

template<typename T>
inline bool Collection<T>::has(Id id) const
{
	const auto idx = collection_id_to_index(id);
	return (idx < array.size() && (array[idx].id == id));
}

template<typename T>
inline T& Collection<T>::get(Id id)
{
	asserts(has(id));
	return array[collection_id_to_index(id)];
}

template<typename T>
inline const T& Collection<T>::get(Id id) const
{
	return const_cast<Collection<T>*>(this)->get(id);
}

template<typename T>
inline bool Collection<T>::id_to_key(Id id, String& out_key) const
{
	const auto idx = collection_id_to_index(id);
	if (idx < keys.size() && keys[idx].hash)
	{
		out_key = keys[idx].name;
		return true;
	}
	out_key.clear();
	return false;
}

template<typename T>
inline bool Collection<T>::key_to_id(const String& key, Id& out_id) const
{
	return key_hash_to_id(key.hash(), out_id);
}

template<typename T>
inline bool Collection<T>::key_hash_to_id(Id hash, Id& out_id) const
{
	asserts(hash);
	for (size_t idx = 0; idx < keys.size(); idx++)
	{
		if (keys[idx].hash == hash)
		{
			out_id = collection_index_to_id(idx);
			asserts(has(out_id)); // technically we don't need check has() since ones unoccupied has hash set to 0, but just to be safe
			return true;
		}
	}
	out_id = 0;
	return false;
}


namespace serialization
{
	template<class T>
	struct CollectionSerializer<Collection<T>>
	{
		template<class TOp>
		static inline void write(TOp& op, const Collection<T>& collection, const String& tag)
		{
			for (const T& elem : collection)
			{
				op.new_object(); // this adds the object prefix
				
				// heading
				op.label(tag);
				String key;
				if (collection.id_to_key(elem.id, key))
				{
					op.string(key);
				}
				op.number(elem.id);
				op.end_heading();
				
				// body
				serialization::serialize(op, elem); // IMPL
				op.newline();
			}
		}

		template<class TOp>
		static inline void read(TOp& op, Collection<T>& collection, const String& tag)
		{
			// TODO: clear
			while (op.next_object())
			{
				Id id{};
				String key;
				T elem;

				// heading
				op.label(tag);
				const bool has_key = (!op.at_eol() && op.opt_string(key));
				const bool has_id = (!op.at_eol() && op.opt_number(id));
				op.end_heading();

				// body
				serialization::serialize(op, elem); // IMPL

				if (has_key && has_id)
				{
					collection.set(id, key, std::move(elem));
				}
				else if (has_key)
				{
					collection.set(key, std::move(elem));
				}
				else
				{
					asserts(has_id && id); // TODO: report error instead of asserts
					collection.set(id, std::move(elem));
				}
			}
		}
	};

	struct NamedIdValue
	{
		template<class TOp, class TId, class TColl>
		static inline void write(TOp& op, const TId& id, const TColl& collection)
		{
			String key;
			const bool has_key = collection.id_to_key(id, key);
			asserts(has_key);
			op.string(key);
		}

		template<class TOp, class TId, class TColl>
		static inline void read(TOp& op, TId& id, const TColl& collection)
		{
			static_assert(!std::is_const_v<TId>);
			String key;
			op.string(key);
			Id full_id;
			const bool found = collection.key_to_id(key, full_id);
			asserts(found);
			asserts(sizeof(TId) >= 8 || (full_id <= (1ull << sizeof(TId) * 8) - 1));
			id = static_cast<TId>(full_id);
		}
	};
}



#pragma once

#include "id.h"
#include "buffer.h"
#include <type_traits>

extern Id new_attr_id();

template<typename T>
class Attribute
{
public:
	using ValueType = T;
	Id id{};
	T default_value{};

	Attribute(const T& default_val):
		id(new_attr_id()),
		default_value(default_val)
	{}
};

struct AttrTableEntry
{
	uint16_t attr_id{};
	uint32_t buffer_ptr{};
};

struct AttrListHandle
{
	uint32_t table_index{};
	uint16_t num_attrs{};
};

struct AttrTable
{
	std::vector<AttrTableEntry> entries;
	Buffer buffer;

	inline void clear();
	template<typename T> const T* get_attr(const AttrListHandle& handle, const Attribute<T>& attr) const;
	template<typename T, typename ValT> void set_attr(AttrListHandle& handle, const Attribute<T>& attr, const ValT& val);
};

namespace attribute_serialization
{
	template<typename T>
	struct can_be_trivially_stored
	{
		static const constexpr bool value = (::std::is_standard_layout_v<T> && ::std::is_trivially_copyable_v<T>);
	};

	template<typename T>
	inline void trivial_load(const Buffer& buffer, size_t ptr, const T*& out_val)
	{
		asserts((ptr + sizeof(T) <= buffer.size()) && buffer.is_aligned(ptr));
		out_val = reinterpret_cast<const T*>(buffer.get(ptr));
	}

	template<typename T>
	inline void trivial_store(Buffer& buffer, const T& val)
	{
		static_assert(alignof(T) <= Buffer::alignment);
		const auto ptr = buffer.size();
		asserts(buffer.is_aligned(ptr));
		buffer.resize(ptr + buffer.get_next_aligned(sizeof(T)));
		std::memcpy(buffer.get(ptr), reinterpret_cast<const void*>(&val), sizeof(T));
	}

	template<typename T>
	inline ::std::enable_if_t<can_be_trivially_stored<T>::value, void>
	load(const Buffer& buffer, size_t ptr, const T*& out_val)
	{
		trivial_load(buffer, ptr, out_val);
	}

	template<typename T>
	inline ::std::enable_if_t<can_be_trivially_stored<T>::value, void>
	store(Buffer& buffer, const T& val)
	{
		trivial_store(buffer, val);
	}
}

inline void AttrTable::clear()
{
	entries.clear();
	buffer.clear();
}

template<typename T>
const T* AttrTable::get_attr(const AttrListHandle& handle, const Attribute<T>& attr) const
{
	const T* val{};
	for (uint16_t i = 0; i < handle.num_attrs; i++)
	{
		auto& table_entry = entries[handle.table_index + i];
		if (table_entry.attr_id == attr.id)
		{
			attribute_serialization::load(buffer, table_entry.buffer_ptr, val);
			break;
		}
	}
	return val;
}

template<typename T, typename ValT> 
void AttrTable::set_attr(AttrListHandle& handle, const Attribute<T>& attr, const ValT& val)
{
	using attr_id_type = decltype(AttrTableEntry::attr_id);
	using buffer_ptr_type = decltype(AttrTableEntry::buffer_ptr);
	using table_index_type = decltype(AttrListHandle::table_index);

	// TODO: consider moving the non-type specific portion of the code below into a separate function
	// so this whole code block doesn't need to be inlined or templated in all instances
	// but on the other hande, everything inlined may be faster

	const auto ptr = buffer.size();
	asserts(buffer.is_aligned(ptr));

	bool found_existing_entry = false;

	// find existing entry for the given attr and point it to the new buffer location
	// TODO: right now we just keep adding new buffer for each set attr value
	// but the buffer may contain multiple values for the same attr, even though
	// it still functions, the values other than the last one are not used and wasting space
	for (uint16_t i = 0; i < handle.num_attrs; i++)
	{
		auto& table_entry = entries[handle.table_index + i];
		if (table_entry.attr_id == attr.id)
		{
			table_entry.buffer_ptr = static_cast<buffer_ptr_type>(ptr);
			found_existing_entry = true;
			break;
			//return buffer;
		}
	}

	if (!found_existing_entry)
	{
		if (handle.num_attrs == 0)
		{
			handle.table_index = static_cast<table_index_type>(entries.size());
			handle.num_attrs = 1;
		}
		else
		{
			// make sure the table for each element is continguous
			// TODO: support setting attribute for previous element somehow
			asserts((handle.table_index + handle.num_attrs) == entries.size());
			handle.num_attrs++;
		}

		auto& table_entry = entries.emplace_back();
		table_entry.attr_id = static_cast<attr_id_type>(attr.id);
		table_entry.buffer_ptr = static_cast<buffer_ptr_type>(ptr);
	}

	const T& store_val = val; // call store using the exact value type, and assuming implicit conversion
	attribute_serialization::store(buffer, store_val);

	// Make sure added value still takes aligned space
	// TODO: consider handle alignment here, i.e. store() can write unaligned amount of bytes, 
	// but after that, code here can fill in unused bytes to make the whole storage aligned
	asserts(buffer.is_aligned(buffer.size()));
}

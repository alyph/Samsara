#pragma once

#include "id.h"
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

namespace attribute_serialization
{
	template<typename T, class BufferT>
	std::enable_if_t<std::is_standard_layout_v<T> && std::is_trivially_copyable_v<T>, void>
	load(const BufferT& buffer, const T*& out_val)
	{
		out_val = reinterpret_cast<const T*>(buffer.peek_bytes(sizeof(T)));
	}

	template<typename T, class BufferT>
	std::enable_if_t<std::is_standard_layout_v<T> && std::is_trivially_copyable_v<T>, void>
	store(BufferT& buffer, const T& val)
	{
		buffer.write_bytes(reinterpret_cast<const void*>(&val), sizeof(T));
	}
}

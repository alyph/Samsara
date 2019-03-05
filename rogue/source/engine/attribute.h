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

struct BufferBlock
{
	uint8_t* ptr{};
	size_t size{};
	operator bool() const { return ptr != nullptr; }
};

namespace attribute_serialization
{
	template<typename T>
	::std::enable_if_t<::std::is_standard_layout_v<T> && ::std::is_trivially_copyable_v<T>, void>
	load(const BufferBlock& buffer, const T*& out_val)
	{
		asserts(sizeof(T) <= buffer.size);
		out_val = reinterpret_cast<const T*>(buffer.ptr);
	}

	template<typename T>
	::std::enable_if_t<::std::is_standard_layout_v<T> && ::std::is_trivially_copyable_v<T>, void>
	store(Buffer& buffer, const T& val)
	{
		static_assert(alignof(T) <= Buffer::alignment);
		const auto ptr = buffer.size();
		asserts(buffer.is_aligned(ptr));
		buffer.resize(ptr + buffer.get_next_aligned(sizeof(T)));
		std::memcpy(buffer.get(ptr), reinterpret_cast<const void*>(&val), sizeof(T));
	}
}

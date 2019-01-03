#pragma once

#include <type_traits>

namespace serialization
{
	template<typename T, class BufferT>
	std::enable_if_t<std::is_standard_layout_v<T> && std::is_trivial_v<T>, void>
	serialize(const T& val, BufferT& buffer)
	{
		buffer.write_bytes(reinterpret_cast<const void*>(&val), sizeof(T));
	}

	template<typename T, class BufferT>
	std::enable_if_t<std::is_standard_layout_v<T> && std::is_trivial_v<T>, void>
	deserialize(T& val, const BufferT& buffer)
	{
		buffer.read_bytes(reinterpret_cast<void*>(&val), sizeof(T));
	}
}


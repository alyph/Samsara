#pragma once

#include <type_traits>

template<typename T>
inline constexpr std::enable_if_t<std::is_enum_v<T>, T> operator&(T a, T b)
{
	return static_cast<T>(static_cast<std::underlying_type_t<T>>(a) & static_cast<std::underlying_type_t<T>>(b));
}

template<typename T>
inline constexpr std::enable_if_t<std::is_enum_v<T>, T> operator|(T a, T b)
{
	return static_cast<T>(static_cast<std::underlying_type_t<T>>(a) | static_cast<std::underlying_type_t<T>>(b));
}

template<typename T>
inline constexpr std::enable_if_t<std::is_enum_v<T>, T&> operator&=(T& a, T b)
{
	return a = static_cast<T>(static_cast<std::underlying_type_t<T>>(a) & static_cast<std::underlying_type_t<T>>(b));
}

template<typename T>
inline constexpr std::enable_if_t<std::is_enum_v<T>, T&> operator|=(T& a, T b)
{
	return a = static_cast<T>(static_cast<std::underlying_type_t<T>>(a) | static_cast<std::underlying_type_t<T>>(b));
}

template<typename T>
inline constexpr std::enable_if_t<std::is_enum_v<T>, bool> has_any(T a, T b)
{
	return (static_cast<std::underlying_type_t<T>>(a) & static_cast<std::underlying_type_t<T>>(b)) != 0;
}

template<typename T>
inline constexpr std::enable_if_t<std::is_enum_v<T>, bool> has_all(T a, T b)
{
	return (static_cast<std::underlying_type_t<T>>(a) & static_cast<std::underlying_type_t<T>>(b)) == static_cast<std::underlying_type_t<T>>(b);
}




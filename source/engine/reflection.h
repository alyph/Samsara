#pragma once
#include <type_traits>

template<class, template <class> class>
struct is_template : public std::false_type {};

template<class T, template <class> class U>
struct is_template<U<T>, U> : public std::true_type {};

template<class A, template <class> class B>
inline constexpr bool is_template_v = is_template<A, B>::value;

template<typename T, typename... Us>
constexpr bool calc_is_any()
{
	return (std::is_same_v<T, Us> || ...);
}

template<typename T, typename... Us>
struct is_any
{
	static const constexpr bool value = calc_is_any<T, Us...>();;
};

template <typename T, typename... Us>
inline constexpr bool is_any_v = is_any<T, Us...>::value;


template<class T, class D>
using defined_or_default_t = std::conditional_t<std::is_void_v<T>, D, T>;

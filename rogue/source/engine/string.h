#pragma once

#include "allocation.h"
#include <cstddef>
#include <cstring>
#include <utility>

struct NormalStringData
{
	AllocHandle alloc_handle;
	uint32_t size;
	uint8_t reserved[4];
};

static const constexpr size_t max_short_string_size = offsetof(NormalStringData, reserved[3]);

struct ShortStringData
{
	uint8_t chars[max_short_string_size];
	uint8_t capacity_byte;
};

static_assert(sizeof(NormalStringData) == sizeof(ShortStringData));

struct StringData
{
	union
	{
		NormalStringData normal_data;
		ShortStringData short_data;
	};

	inline StringData() { std::memset(this, 0, sizeof(StringData)); }
	
	inline const char* c_str() const;
	inline size_t size() const;	
};

template<bool Temp>
class StringBase
{
public:
	StringData str_data;

	inline StringBase() = default;
	inline StringBase(const char* str) { *this = str; }
	template<bool OtherTemp>
	inline StringBase(const StringBase<OtherTemp>& other) { *this = other; }
	inline StringBase(String&& other) { *this = std::move(other); }
	inline ~StringBase();

	inline StringBase& operator=(const char* str);
	template<bool OtherTemp>
	inline StringBase& operator=(const StringBase<OtherTemp>& other);
	inline StringBase& operator=(StringBase&& other);

	inline const char* c_str() const { return str_data.c_str(); }
	inline size_t size() const { return str_data.size(); }	

private:
	inline void assign(const char* str);
	inline void add_ref();
	inline void release_ref();
};

using String = StringBase<false>;
using TempString = StringBase<true>;

// StringData


// String

template<bool Temp>
inline StringBase<Temp>::~StringBase()
{
	release_ref();
}

template<bool Temp>
inline StringBase<Temp>& StringBase<Temp>::operator=(const char* str)
{
	assign(str);
	return *this;
}

template<bool Temp>
template<bool OtherTemp>
inline StringBase<Temp>& StringBase<Temp>::operator=(const StringBase<OtherTemp>& other)
{
	if constexpr (Temp == OtherTemp)
	{
		release_ref();
		other.add_ref();
		str_data = other.str_data;
	}
	else
	{
		assign(other.c_str());
	}	
	return *this;
}

template<bool Temp>
inline StringBase<Temp>& StringBase<Temp>::operator=(StringBase<Temp>&& other)
{
	std::swap(str_data, other.str_data);
}

template<bool Temp>
inline void StringBase<Temp>::assign(const char* str)
{
	release_ref();

	const auto len = std::strlen(str);
	if (len <= max_short_string_size)
	{

	}
	else
	{
	}
}

template<bool Temp>
inline void StringBase<Temp>::add_ref()
{

}

template<bool Temp>
inline void StringBase<Temp>::release_ref()
{

}






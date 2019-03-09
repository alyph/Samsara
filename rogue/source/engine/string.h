#pragma once

#include "allocation.h"
#include "engine.h"
#include <cstddef>
#include <cstring>
#include <utility>
#include <algorithm>

static const constexpr unsigned int string_layout_bits = 2;
static const constexpr unsigned int short_string_capacity_bits = (8 - string_layout_bits);
//static const constexpr unsigned int string_start_bits = (sizeof(NormalStringData::start) * 8 - string_layout_bits);
static const constexpr size_t string_grow_factor = 2;
static const constexpr size_t string_max_grow = (4 * 1024);

struct NormalStringData
{
	AllocHandle alloc_handle;
	uint32_t size;
	uint32_t start : (32 - string_layout_bits);
	uint32_t layout: string_layout_bits;
};

static const constexpr size_t max_short_string_size = (sizeof(AllocHandle) + sizeof(NormalStringData::size) + 3);

struct ShortStringData
{
	uint8_t chars[max_short_string_size];
	uint8_t remaining_capacity: (8 - string_layout_bits);
	uint8_t layout: string_layout_bits;
};


static_assert(sizeof(NormalStringData) == sizeof(ShortStringData));
static_assert(sizeof(NormalStringData) == 24);
static_assert(max_short_string_size < (1 << short_string_capacity_bits));

enum class StringLayout : uint8_t
{
	short_string = 0,
	normal_string = 1,
	sub_string = 2,
	max,
};

static_assert(static_cast<uint8_t>(StringLayout::max) <= (1 << string_layout_bits));


struct StringHeader
{
	//uint32_t capacity{};
	uint16_t ref_count{};
	bool ref_by_view{};
};

struct StringData
{
	union
	{
		NormalStringData normal_data;
		ShortStringData short_data;
	};

	inline StringData() { clear(); }	
	inline void clear();
	inline const char* c_str() const;
	inline size_t size() const;
	inline StringLayout layout() const;
	inline bool is_short() const { return (layout() == StringLayout::short_string); }
	inline bool is_normal() const { return (layout() == StringLayout::normal_string); }
	inline StringHeader* header() const;
};

class StringView final
{
public:
	StringData str_data;

	inline const char* c_str() const { return str_data.c_str(); }
	inline size_t size() const { return str_data.size(); }	
};

template<bool Temp>
class StringBase final
{
public:
	StringData str_data;

	inline StringBase() = default;
	template<size_t N>
	inline StringBase(const char (&str)[N]);
	inline StringBase(const StringBase<Temp>& other) { *this = other; }
	inline explicit StringBase(const StringBase<!Temp>& other) { *this = other; }
	inline StringBase(StringBase<Temp>&& other) { *this = std::move(other); }
	inline ~StringBase();

	template<size_t N>
	inline StringBase& operator=(const char (&str)[N]);
	inline StringBase& operator=(const StringBase<Temp>& other);
	inline StringBase& operator=(const StringBase<!Temp>& other);
	inline StringBase& operator=(StringBase<Temp>&& other);

	inline operator const StringView&() const;

	inline const char* c_str() const { return str_data.c_str(); }
	inline size_t size() const { return str_data.size(); }	

private:
	void assign(const char* str);
};

using String = StringBase<false>;
using TempString = StringBase<true>;

// StringData

inline void StringData::clear()
{
	short_data.chars[0] = 0;
	short_data.remaining_capacity = max_short_string_size;
	short_data.layout = 0;
}

inline const char* StringData::c_str() const
{
	if (is_short())
	{
		return reinterpret_cast<const char*>(short_data.chars);
	}
	else if (is_normal())
	{
		auto ptr = reinterpret_cast<uint8_t*>(normal_data.alloc_handle.get(engine().allocators));
		asserts(ptr);
		return reinterpret_cast<const char*>(ptr + sizeof(StringHeader));
	}
	
	asserts(false); // other string layout cannot return c_str()
	return nullptr;	
}

inline size_t StringData::size() const
{
	return is_short() ? (max_short_string_size - short_data.remaining_capacity) : normal_data.size;
}

inline StringLayout StringData::layout() const
{
	return static_cast<StringLayout>(short_data.layout);
}

inline StringHeader* StringData::header() const
{
	if (is_short())
	{
		return nullptr;
	}

	auto header = reinterpret_cast<StringHeader*>(normal_data.alloc_handle.get(engine().allocators));
	asserts(header);
	return header;
}



template<bool Temp>
inline void add_string_ref(const StringData& str_data)
{
	auto header = str_data.header();
	if (header)
	{
		header->ref_count++;
		asserts(header->ref_count > 1);
	}
}

template<bool Temp>
inline void release_string_ref(StringData& str_data)
{
	auto header = str_data.header();
	if (header)
	{
		asserts(header->ref_count >= 1);
		header->ref_count--;
		if (header->ref_count == 0)
		{
			engine().allocators.deallocate(str_data.normal_data.alloc_handle);
		}
		str_data.clear();
	}
}

// TODO: we don't need this to be a function, put it back into the caller
template<bool Temp>
inline bool reuse_allocation_on_write(const StringData& str_data)
{
	// if header is null, then it's short string and has no existing allocation (thus cannot be reused)
	auto header = str_data.header();

	// we can resuse the allocation if:
	// only referenced by 1 (the current string itself), so the modification won't be reflected in other strings
	// not referenced by StringView, since StringView does not keep track of ref count, we do not want the change reflected in any views as well
	return (header && header->ref_count == 1 && !header->ref_by_view);
}

// String

template<bool Temp>
template<size_t N>
inline StringBase<Temp>::StringBase(const char (&str)[N])
{
	assign(str);
}

template<bool Temp>
inline StringBase<Temp>::~StringBase()
{
	release_string_ref<Temp>(str_data);
}

template<bool Temp>
template<size_t N>
inline StringBase<Temp>& StringBase<Temp>::operator=(const char (&str)[N])
{
	assign(str);
	return *this;
}

template<bool Temp>
inline StringBase<Temp>& StringBase<Temp>::operator=(const StringBase<Temp>& other)
{
	add_string_ref<Temp>(other.str_data);
	release_string_ref<Temp>(str_data);
	str_data = other.str_data;
	return *this;
}

template<bool Temp>
inline StringBase<Temp>& StringBase<Temp>::operator=(const StringBase<!Temp>& other)
{
	assign(other.c_str());
	return *this;
}

template<bool Temp>
inline StringBase<Temp>& StringBase<Temp>::operator=(StringBase<Temp>&& other)
{
	std::swap(str_data, other.str_data);
}

template<bool Temp>
inline StringBase<Temp>::operator const StringView&() const
{
	return *reinterpret_cast<const StringView*>(this);
}

// TODO: we should take both a const char* ptr and the length, so a string view can be assigned
template<bool Temp>
void StringBase<Temp>::assign(const char* str)
{
	const auto len = std::strlen(str);
	if (len <= max_short_string_size)
	{
		release_string_ref<Temp>(str_data);
		std::memcpy(&str_data, str, len + 1);
		str_data.short_data.remaining_capacity = (max_short_string_size - len);
		str_data.short_data.layout = static_cast<uint8_t>(StringLayout::short_string);
	}
	else if (reuse_allocation_on_write<Temp>(str_data))
	{
		AllocatorGlobals& alloc_globals = engine().allocators;
		size_t alloc_size = sizeof(StringHeader) + len + 1;
		if (len > str_data.normal_data.size && 
			alloc_size > str_data.normal_data.alloc_handle.capacity(alloc_globals))
		{
			// TODO: this should be a unique reallocation as no other string or stringView would reference it 
			// so an optimization would be the allocator does not need bump its min valid id nor giving out new reg id
			const size_t new_size = std::min(alloc_size * string_grow_factor, alloc_size + string_max_grow);
			alloc_globals.reallocate(str_data.normal_data.alloc_handle, new_size);
		}

		auto ptr = reinterpret_cast<uint8_t*>(str_data.normal_data.alloc_handle.get(alloc_globals));
		std::memcpy((ptr + sizeof(StringHeader)), str, len + 1);

		str_data.normal_data.size = static_cast<decltype(str_data.normal_data.size)>(len);
	}
	else
	{
		// TODO: maybe check if the str is the same, to avoid an extra copy

		release_string_ref<Temp>(str_data);
		size_t alloc_size = sizeof(StringHeader) + len + 1; // allocate strict size for the first time since majority of the time it will just get referenced around and not rewritten because of copy on write rule
		AllocatorGlobals& alloc_globals = engine().allocators;

		Allocator allocator = Allocator::string;
		if constexpr (Temp)
		{
			allocator = alloc_globals.current_temp_allocator;
		}
		
		str_data.normal_data.alloc_handle = alloc_globals.allocate(allocator, alloc_size);
		
		auto ptr = reinterpret_cast<uint8_t*>(str_data.normal_data.alloc_handle.get(alloc_globals));
		auto header = reinterpret_cast<StringHeader*>(ptr);
		//header->capacity = len;
		header->ref_count = 1;
		std::memcpy((ptr + sizeof(StringHeader)), str, len + 1);

		str_data.normal_data.size = static_cast<decltype(str_data.normal_data.size)>(len);
		str_data.normal_data.start = 0;
		str_data.normal_data.layout = static_cast<uint8_t>(StringLayout::normal_string);
	}
}






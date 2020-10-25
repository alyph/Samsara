#pragma once

#include "allocation.h"
#include "engine.h"
#include "id.h"
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <utility>
#include <algorithm>
#include <string_view>
#include <functional>

#define STRING_REF_COUNT 0
#define STRING_VIEW STRING_REF_COUNT

// Strings are immutable
// By default all Strings are allocated in temp memory
// Use store() to store and allocate the given string in a persistent memory
// Persistently stored Strings are ref counted and automatically deallocated when no longer referenced
// Temporarily stored Strings are not ref counted and only valid for a reasonable amount of time defined 
// by the application (usually until next frame). References to these Strings should not be kept, but 
// should be safe to pass into or return from function calls 
//
// StringView is a trivially copiable references to a String. They are not ref counted and valid as long as
// the referenced Strings are valid. Generally it can be considered as safe and valid for the same life
// time as a temporarily allocated String. Even if a String (persistent) is deallocated, the String buffer
// can still be accessed until the memory is reclaimed (which is normally until next frame)


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

struct LiteralStringData
{
	const char* ptr;
	uint64_t padding;
	uint32_t size;
	uint32_t start : (32 - string_layout_bits);
	uint32_t layout: string_layout_bits;
};

static_assert(sizeof(LiteralStringData) == sizeof(NormalStringData));
static_assert(alignof(LiteralStringData) == alignof(NormalStringData));


enum class StringLayout : uint8_t
{
	short_string = 0,
	normal_string = 1,
	literal = 2,
	max,
};

static_assert(static_cast<uint8_t>(StringLayout::max) <= (1 << string_layout_bits));

// TODO: since this is only used by StringStore, one optimization would be not adding a header for normal String
struct StringHeader
{
	uint32_t ref_count{};
};

struct StringData
{
	union
	{
		NormalStringData normal_data;
		LiteralStringData literal_data;
		ShortStringData short_data;
	};

	inline StringData() { clear(); }	
	inline void clear();
	inline const char* data() const;
	inline char* data_mut() { return const_cast<char*>(data()); }
	inline size_t size() const;
	inline const char* c_str() const;
	inline StringLayout layout() const;
	inline bool is_short() const { return (layout() == StringLayout::short_string); }
	inline bool is_normal() const { return (layout() == StringLayout::normal_string); }
	inline bool is_literal() const { return (layout() == StringLayout::literal); }
	inline bool is_persistent_allocated() const { return is_normal() && normal_data.alloc_handle.allocator_type() < Allocator::temp; }
	inline bool is_temp_allocated() const { return is_normal() && normal_data.alloc_handle.allocator_type() >= Allocator::temp; }
	inline StringHeader* header() const;
	inline bool equal(const StringData& other) const;
};

// value always temp
// const ref may be persistent
class String final
{
public:
	StringData str_data;

	inline String() = default;
	template<size_t N>
	inline String(const char (&str)[N]);
	template<size_t N>
	inline String(char (&str)[N]) = delete;
	inline String(const char* start, size_t size);
#if STRING_REF_COUNT
	inline String(const String& other) { *this = other; }
	inline String(String&& other);
	inline ~String();
#endif

	template<size_t N>
	inline String& operator=(const char (&str)[N]);
	template<size_t N>
	inline String& operator=(char (&str)[N]) = delete;
#if STRING_REF_COUNT
	inline String& operator=(const String& other);
	inline String& operator=(String&& other);
#endif

	inline void store();
	template<size_t N>
	inline void store(const char (&str)[N]);
	template<size_t N>
	inline void store(char (&str)[N]) = delete;
	inline void store(const String& str);

	inline bool operator==(const String& other) const { return str_data.equal(other.str_data); }
	inline bool operator!=(const String& other) const { return !str_data.equal(other.str_data); }
	inline const char* c_str() const { return str_data.c_str(); }
	inline bool empty() const { return size() == 0; }
	inline size_t size() const { return str_data.size(); }
	inline const char* data() const { return str_data.data(); }
	inline Id hash() const;
	inline void clear() { str_data.clear(); }

#if STRING_REF_COUNT
private:
	inline void dispose();
#endif
};

inline String make_string_view(const char* start, size_t size);

#if STRING_VIEW
class StringView final
{
public:
	StringData str_data;
	inline StringView() = default;
	// inline StringView(const StringView& other) = default;
	inline StringView(const String& str) { str_data = str.str_data; }
	inline StringView& operator=(const String& str) { str_data = str.str_data; return *this; }
	inline const String& str() const { return *reinterpret_cast<const String*>(this); }
};
#endif

class StringBuilder final
{
public:
	template<typename ...Ts>
	inline void append_format(const String& fmt, const Ts&... args);
	inline String to_str();
private:
	StringData str_data;
};

template<typename ...Ts>
inline String format_str(const String& fmt, const Ts&... args);
inline String cstr_to_str(const char* cstr); // TODO: maybe should impl const char* String constructor and assignment instead?

// string assignment

inline void assign_short_string(StringData& str_data, const char* start, size_t size)
{
	std::memcpy(&str_data, start, size);
	str_data.short_data.chars[size] = 0; // end of string marker		
	str_data.short_data.remaining_capacity = (max_short_string_size - size);
	str_data.short_data.layout = static_cast<uint8_t>(StringLayout::short_string);
}

// TODO: probably don't want to inline this
inline void assign_normal_string(StringData& str_data, const char* start, size_t size, Allocator allocator)
{
	// allocate strict size for the first time since majority of the time it will just 
	// get referenced around and not rewritten because of copy on write rule
	// for strings that get assigned multiple times, there can be optimization to allocate
	// extra capacity to avoid constant reallocation (see in assign_string_store())
	size_t alloc_size = sizeof(StringHeader) + size + 1; 
	AllocatorGlobals& alloc_globals = engine().allocators;
	
	auto handle = alloc_globals.allocate(allocator, alloc_size);
	auto ptr = reinterpret_cast<uint8_t*>(handle.ptr);
	auto header = reinterpret_cast<StringHeader*>(ptr);

	// TODO: remove this if we are not doing ref counted strings
	header->ref_count = 0;
	auto buffer = (ptr + sizeof(StringHeader));
	std::memcpy(buffer, start, size);
	buffer[size] = 0;

	str_data.normal_data.alloc_handle = handle;
	str_data.normal_data.size = static_cast<decltype(str_data.normal_data.size)>(size);
	str_data.normal_data.start = 0;
	str_data.normal_data.layout = static_cast<uint8_t>(StringLayout::normal_string);
}

inline void assign_temp_string(StringData& str_data, const char* start, size_t size)
{
	if (size <= max_short_string_size)
	{
		assign_short_string(str_data, start, size);
	}
	else
	{
		assign_normal_string(str_data, start, size, engine().allocators.current_temp_allocator);
	}
}

inline void assign_stored_string(StringData& str_data, const char* start, size_t size)
{
	if (size <= max_short_string_size)
	{
		assign_short_string(str_data, start, size);
	}
	else
	{
		// TODO: maybe we should force user to select an explicit allocator instead
			assign_normal_string(str_data, start, size, perm_allocator());
		str_data.header()->ref_count = 1;
	}
}

inline void assign_string_literal(StringData& str_data, const char* start, size_t size)
{
	if (size <= max_short_string_size)
	{
		assign_short_string(str_data, start, size);
	}
	else
	{
		str_data.literal_data.ptr = start;
		str_data.literal_data.padding = 0;
		str_data.literal_data.size = static_cast<decltype(str_data.literal_data.size)>(size);
		str_data.literal_data.start = 0;
		str_data.normal_data.layout = static_cast<uint8_t>(StringLayout::literal);
	}
}

inline void* validate_and_get_string_buffer(const StringData& str_data)
{
	auto ptr = str_data.normal_data.alloc_handle.get();
	asserts(ptr);
	return ptr;
}

#if STRING_REF_COUNT
inline uint32_t string_ref_count(const StringData& str_data)
{
	auto header = str_data.header();
	return (header ? header->ref_count : 0);
}

inline void add_string_ref(const StringData& str_data)
{
	auto header = str_data.header();
	if (header)
	{
		header->ref_count++;
		asserts(header->ref_count > 1);
	}
}

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
#endif


// StringData

inline void StringData::clear()
{
	short_data.chars[0] = 0;
	short_data.remaining_capacity = max_short_string_size;
	short_data.layout = 0;
}

inline const char* StringData::data() const
{
	if (is_short())
	{
		return reinterpret_cast<const char*>(short_data.chars);
	}
	else if (is_literal())
	{
		return (literal_data.ptr + literal_data.start);
	}
	else
	{
		auto ptr = reinterpret_cast<uint8_t*>(validate_and_get_string_buffer(*this));
		return reinterpret_cast<const char*>(ptr + sizeof(StringHeader) + normal_data.start);
	}
}

inline size_t StringData::size() const
{
	// NOTE: literal and normal string's size field is at the same position
	return is_short() ? (max_short_string_size - short_data.remaining_capacity) : normal_data.size;
}

inline const char* StringData::c_str() const
{
	const auto ptr = data();
	const auto len = size();
	if (*(ptr + len) == 0)
	{
		return ptr;
	}
	else
	{
		// partial string
		// must copy into a temp storage and make it 0 terminated
		StringData temp_str;
		assign_temp_string(temp_str, ptr, len);
		return temp_str.c_str(); // this is safe since any allocation will last at least until the end of current calling context
	}
}

inline StringLayout StringData::layout() const
{
	return static_cast<StringLayout>(short_data.layout);
}

inline StringHeader* StringData::header() const
{
	return is_normal() ? reinterpret_cast<StringHeader*>(validate_and_get_string_buffer(*this)) : nullptr;
}

inline bool StringData::equal(const StringData& other) const
{
	const auto s = size();
	if (s != other.size())
	{
		return false;
	}

	auto this_data = data();
	auto other_data = other.data();
	if (this_data == other_data)
	{
		return true;
	}

	return (std::memcmp(this_data, other_data, s) == 0);
}


// String

template<size_t N>
inline String::String(const char (&str)[N])
{
	assign_string_literal(str_data, str, (N - 1));
}

inline String::String(const char* start, size_t size)
{
	assign_temp_string(str_data, start, size);
}

#if STRING_REF_COUNT
inline String::String(String&& other)
{
	std::swap(str_data, other.str_data);
}

inline String::~String()
{
	dispose();
}
#endif

template<size_t N>
inline String& String::operator=(const char (&str)[N])
{
#if STRING_REF_COUNT	
	dispose();
#endif
	assign_string_literal(str_data, str, (N - 1));
	return *this;
}

#if STRING_REF_COUNT
inline String& String::operator=(const String& other)
{
	if (other.str_data.is_persistent_allocated())
	{
		add_string_ref(other.str_data);
	}
	dispose();
	str_data = other.str_data;
	return *this;
}

inline String& String::operator=(String&& other)
{
	std::swap(str_data, other.str_data);
	return *this;
}
#endif

inline void String::store()
{
	if (str_data.is_temp_allocated())
	{
		// TODO: maybe just call assign_normal_string()??
		assign_stored_string(str_data, str_data.data(), str_data.size());
	}
}

template<size_t N>
inline void String::store(const char (&str)[N])
{
#if STRING_REF_COUNT	
	dispose();
#endif
	assign_string_literal(str_data, str, (N - 1));
}

inline void String::store(const String& str)
{
	*this = str;
	store();
}

inline Id String::hash() const
{
	std::string_view sv{data(), size()};
	return std::hash<std::string_view>{}(sv);
}

inline String make_string_view(const char* start, size_t size)
{
	String str;
	assign_string_literal(str.str_data, start, size);
	return str;
}

#if STRING_REF_COUNT
inline void String::dispose()
{
	if (str_data.is_persistent_allocated())
	{
		release_string_ref(str_data);
	}
}
#endif

// StringBuilder

template<typename T>
inline std::enable_if_t<std::is_arithmetic_v<T> || std::is_pointer_v<T> || std::is_enum_v<T>, T>
format_arg_wrapper(const T& arg)
{
	return arg;
}

inline const char* format_arg_wrapper(const String& arg)
{
	return arg.c_str();
}

inline char* sb_buffer_end(StringData& str_data)
{
	return str_data.data_mut() + str_data.size();
}

inline size_t sb_buffer_remaining(const StringData& str_data)
{
	if (str_data.is_short())
	{
		return str_data.short_data.remaining_capacity + 1; // add the null terminator slot
	}
	else
	{
		const auto& normal_data = str_data.normal_data;
		// TODO: maybe we could cache this capacity during sb_expand_buffer()
		// to avoid constantly querying the alloc header
		const auto capacity = normal_data.alloc_handle.capacity(engine().allocators);
		asserts(capacity > 0);
		const auto occupied = normal_data.start + normal_data.size;
		return (capacity - sizeof(StringHeader) - occupied);
	}
}

// size not including null terminator
inline void sb_increment_size(StringData& str_data, size_t size)
{
	if (str_data.is_short())
	{
		str_data.short_data.remaining_capacity -= (uint8_t)size;
	}
	else
	{
		str_data.normal_data.size += (uint32_t)size;
	}
}

// size including null terminator
extern void sb_expand_buffer(StringData& str_data, size_t size);

template<typename ...Ts>
inline void StringBuilder::append_format(const String& fmt, const Ts&... args)
{
	// TODO: real implementation of string formatting
	// similar to fmt lib and c++20 std::format
	// https://github.com/fmtlib/fmt
	// https://fmt.dev/Text%20Formatting.html
	auto end = sb_buffer_end(str_data);
	auto buffer_size = sb_buffer_remaining(str_data);
	auto orig_remaining = str_data.short_data.remaining_capacity;
	int str_size = std::snprintf(end, buffer_size, fmt.c_str(), format_arg_wrapper(args)...);
	if (str_size >= buffer_size)
	{
		// no need to check if it's short string
		// because if it's normal string the str_data itself shouldn't change
		// but for short string, all data will be modified by the previous snprintf
		str_data.short_data.remaining_capacity = orig_remaining;
		sb_expand_buffer(str_data, str_size + 1);
		end = sb_buffer_end(str_data);
		std::snprintf(end, str_size + 1, fmt.c_str(), format_arg_wrapper(args)...);
	}
	sb_increment_size(str_data, str_size);
}

inline String StringBuilder::to_str()
{
	// TODO: maybe we should shrink before sending to string
	// this way not only it saves memory, but also enforces the
	// rule where the size of the allocated memory is always
	// sizeof(StringHeader) + str_size + 1, and then we can rely
	// on this convention to check if a string is a sub string
	String str;
	str.str_data = str_data;
	str_data.clear();
	return str;
}





// functions

template<typename ...Ts>
inline String format_str(const String& fmt, const Ts&... args)
{
	StringBuilder builder;
	builder.append_format(fmt, args...);
	return builder.to_str();
}

inline String cstr_to_str(const char* cstr)
{
	return String{cstr, std::strlen(cstr)};
}


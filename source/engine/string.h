#pragma once

#include "allocation.h"
#include "engine.h"
#include "attribute.h"
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <utility>
#include <algorithm>

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

// TODO: maybe another layout would be pointing directly to a const char* of a string literal
enum class StringLayout : uint8_t
{
	short_string = 0,
	normal_string = 1,
	sub_string = 2, // TODO: do we still need sub string?
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
	inline bool is_sub() const { return (layout() == StringLayout::sub_string); }
	inline bool is_persistent_allocated() const { return !is_short() && normal_data.alloc_handle.allocator_type() == Allocator::string; }
	inline StringHeader* header() const;
};

#if 0
class StringAccessor
{
public:
	StringData str_data;

	inline const char* c_str() const { return str_data.c_str(); }
	inline size_t size() const { return str_data.size(); }	
};
#endif

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
	inline String(const String& other) { *this = other; }
	inline String(String&& other);
	inline ~String();

	template<size_t N>
	inline String& operator=(const char (&str)[N]);
	template<size_t N>
	inline String& operator=(char (&str)[N]) = delete;
	inline String& operator=(const String& other);
	inline String& operator=(String&& other);

	inline void store();
	template<size_t N>
	inline void store(const char (&str)[N]);
	template<size_t N>
	inline void store(char (&str)[N]) = delete;
	inline void store(const String& str);

	inline const char* c_str() const { return str_data.c_str(); }
	inline size_t size() const { return str_data.size(); }

private:
	inline void dispose();
};

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


#if 0

// can be temp or persistent
// TODO: even though this type is mostly returned when getting a portion of the existing String (or StringStore),
// using functions like (substr(), left(), right(), trim() etc.), the content may not always be a "sub" string
// (i.e. is_sub() returns true), for example trim() may return the whole string, since there may not be anything
// trim. So the class name may be mis-leading, more accurately it should be named something like TempOrPersistString!
class SubString final : public StringAccessor
{
	inline SubString() = default;
	inline SubString(const SubString& other) = delete; // probably no need to copy
	inline SubString(SubString&& other) { *this = std::move(other); }
	inline ~SubString();
	
	inline SubString& operator=(const SubString& other) = delete; // probably no need to copy
	inline SubString& operator=(SubString&& other);

	inline operator const String&() const { return *reinterpret_cast<const String*>(this); }
};

// always persistent
class StringStore final : public StringAccessor
{
public:
	inline StringStore() = default;
	template<size_t N>
	inline StringStore(const char (&str)[N]);
	inline StringStore(const String& str) { *this = str; }
	inline StringStore(const SubString& str) { *this = str; }
	inline StringStore(const StringStore& other) { *this = other; }
	inline StringStore(StringStore&& other) { *this = std::move(other); }
	inline ~StringStore();

	template<size_t N>
	inline StringStore& operator=(const char (&str)[N]);
	inline StringStore& operator=(const String& str);
	inline StringStore& operator=(const SubString& str);
	inline StringStore& operator=(const StringStore& other);
	inline StringStore& operator=(StringStore&& other);

	inline operator const String&() const { return *reinterpret_cast<const String*>(this); }

	// inline String to_string_copied() const;
	// inline String to_string_latched() const; // should make this implicit maybe just do operator const String&
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

#endif


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
		assign_normal_string(str_data, start, size, Allocator::string);
		str_data.header()->ref_count = 1;
	}
}

inline void* validate_and_get_string_buffer(const StringData& str_data)
{
	auto ptr = str_data.normal_data.alloc_handle.get(engine().allocators);
	asserts(ptr);

	// latched String or StringStore, must make sure the buffer is still somewhat valid
	// e.g. size matches (we won't be validate content doesn't change, as long as the size match, 
	// nothing horrible will be happening)
	// reason why we must validate is because unlike String a StringStore may reuse the 
	// allocated block to place in the new string content or move the whole block to new location
	// via reallocation
	// if (str_data.normal_data.alloc_handle.allocator_type() == Allocator::string)
	// {
	// 	auto header = reinterpret_cast<const StringHeader*>(ptr);
	// 	asserts((str_data.is_sub() ? 
	// 		(str_data.normal_data.start + str_data.normal_data.size <= header->size) : 
	// 		(str_data.normal_data.size == header->size)));		
	// }

	return ptr;
}

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
	else
	{
		auto ptr = reinterpret_cast<uint8_t*>(validate_and_get_string_buffer(*this));
		return reinterpret_cast<const char*>(ptr + sizeof(StringHeader) + normal_data.start);
	}
}

inline size_t StringData::size() const
{
	return is_short() ? (max_short_string_size - short_data.remaining_capacity) : normal_data.size;
}

inline const char* StringData::c_str() const
{
	if (is_sub())
	{
		// if the sub string ends at the end of full string, can safely return it
		if (*(data() + size()) == 0)
		{
			return data();
		}
		else
		{
			// expensive operation, create a temp string and return its c_str()
			StringData temp_str;
			assign_normal_string(temp_str, data(), size(), engine().allocators.current_temp_allocator);
			return temp_str.c_str(); // this is safe since any allocation will last at least until the end of current calling context
		}
	}
	else
	{
		return data();
	}	
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

	auto header = reinterpret_cast<StringHeader*>(validate_and_get_string_buffer(*this));
	return header;
}


// String

template<size_t N>
inline String::String(const char (&str)[N])
{
	assign_temp_string(str_data, str, (N - 1));
}

inline String::String(const char* start, size_t size)
{
	assign_temp_string(str_data, start, size);
}

inline String::String(String&& other)
{
	std::swap(str_data, other.str_data);
}

inline String::~String()
{
	dispose();
}

template<size_t N>
inline String& String::operator=(const char (&str)[N])
{
	dispose();
	assign_temp_string(str_data, str, (N - 1));
	return *this;
}

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

inline void String::store()
{
	if (!str_data.is_short() && !str_data.is_persistent_allocated())
	{
		assign_stored_string(str_data, str_data.data(), str_data.size());
	}
}

template<size_t N>
inline void String::store(const char (&str)[N])
{
	dispose();
	assign_stored_string(str_data, str, (N - 1));
}

inline void String::store(const String& str)
{
	*this = str;
	store();
}


inline void String::dispose()
{
	if (str_data.is_persistent_allocated())
	{
		release_string_ref(str_data);
	}
}

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


#if 0

// SubString
inline SubString::~SubString()
{
	if (str_data.is_persistent_allocated())
	{
		release_string_ref(str_data);
	}
}

inline SubString& SubString::operator=(SubString&& other)
{
	std::swap(str_data, other.str_data);
	return *this;
}


// StringStore


inline void assign_string_store(StringData& str_data, const char* start, size_t size)
{
	if (size <= max_short_string_size)
	{
		release_string_ref(str_data);
		assign_short_string(str_data, start, size);
	}
	else if (string_ref_count(str_data) == 1)
	{
		// single self reference, we can change the content of the string freely

		AllocatorGlobals& alloc_globals = engine().allocators;
		size_t alloc_size = sizeof(StringHeader) + size + 1;
		if (size > str_data.normal_data.size && 
			alloc_size > str_data.normal_data.alloc_handle.capacity(alloc_globals))
		{
			// TODO: this should be a unique reallocation as no other StringStore would reference it 
			//  ( NOTE: String would not share copy of this, as String can only hold a const ref of the StringStore itself, 
			//    so any change to the StringStore would apply to the String as well)
			// so an optimization would be the allocator does not need bump its min valid id nor giving out new reg id
			const size_t new_size = std::min(alloc_size * string_grow_factor, alloc_size + string_max_grow);
			alloc_globals.reallocate(str_data.normal_data.alloc_handle, new_size);
		}

		auto ptr = reinterpret_cast<uint8_t*>(str_data.normal_data.alloc_handle.get(alloc_globals));
		auto buffer = (ptr + sizeof(StringHeader));
		std::memcpy(buffer, start, size);
		buffer[size] = 0;

		str_data.normal_data.size = static_cast<decltype(str_data.normal_data.size)>(size);
	}
	else
	{
		// TODO: maybe check if the str is the same, to avoid an extra copy

		release_string_ref(str_data);
		assign_normal_string(str_data, start, size, Allocator::string);

		// persistent stored string starts with ref count 1 (default string starts and remains at 0)
		str_data.header()->ref_count = 1;
	}
}

template<size_t N>
inline StringStore::StringStore(const char (&str)[N])
{
	assign_string_store(str_data, str, (N - 1));
}

inline StringStore::~StringStore()
{
	release_string_ref(str_data);
}

template<size_t N>
inline StringStore& StringStore::operator=(const char (&str)[N])
{
	assign_string_store(str_data, str, (N - 1));
	return *this;
}

inline StringStore& StringStore::operator=(const String& str)
{
	// since str is const ref, it can be either String, SubString or StringStore
	// perform copy if it is StringStore by checking if it is normal and persistently allocated
	if (str.str_data.is_normal() && str.str_data.is_persistent_allocated())
	{
		add_string_ref(str.str_data);
		release_string_ref(str_data);
		str_data = str.str_data;
	}
	else
	{
		assign_string_store(str_data, str.str_data.data(), str.str_data.size());
	}
	return *this;
}

inline StringStore& StringStore::operator=(const SubString& str)
{
	// NOTE: even though the str is a SubString type, it doesn't mean it's always sub string
	// (i.e. is_sub() returns true). The SubString only represents a string that can be either 
	// temporarily or persistently allocated string, but doesn't mean it may not be a full string
	// (see comments for SubString class above)
	if (str.str_data.is_normal() && str.str_data.is_persistent_allocated())
	{
		add_string_ref(str.str_data);
		release_string_ref(str_data);
		str_data = str.str_data;
	}
	else
	{
		assign_string_store(str_data, str.str_data.data(), str.str_data.size());
	}
	return *this;
}

inline StringStore& StringStore::operator=(const StringStore& other)
{
	add_string_ref(other.str_data);
	release_string_ref(str_data);
	str_data = other.str_data;
	return *this;
}

inline StringStore& StringStore::operator=(StringStore&& other)
{
	std::swap(str_data, other.str_data);
	return *this;
}


namespace attribute_serialization
{
	inline void load(const Buffer& buffer, size_t ptr, const String*& out_val)
	{
		trivial_load(buffer, ptr, out_val);
	}

	inline void store(Buffer& buffer, const String& val)
	{
		if (val.str_data.is_persistent_allocated())
		{
			// if persistently allocated, must store it into a temp string
			// that's because we do not keep reference and the persistent string
			// may change or released because not ref counted
			String temp = val;
			trivial_store(buffer, temp);
		}
		else
		{
			trivial_store(buffer, val);
		}
	}
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




#endif

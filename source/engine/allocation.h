#pragma once

#include "assertion.h"
#include "buffer.h"
#include <cstdint>
#include <vector>

// Design considerations:
// - allocators give out memories (pointer + num of bytes)
// - memory allocated by allocators are always valid (as allocators never move, or release the memory back to OS)
// - allocated memories are gone and never reclaimed
//    - it's not required for user to deallocate the memory (no deallocate function provided)
//    - users can re-allocate to get a new block of memory
//    - entire memory from an allocator will be released (and thus can be allocated again) as a whole based on its own specific timing
// - each call to allocate memory must specify which allocator to use
// - there are a few globally tagged generic allocators:
//    - system allocator (used by system and the memeoy last the whole life time of the app)
//    - app allocator (used by user and most likely last the whole life time of the app, but user will determine when to release)
//    - stage allocator (used by user code, persistent across long time span, but may be released whenever user see fit)
//    - temp allocator (used by user or system code, only valid for one frame, or any short interval that the user designates)
// - custom allocators can be requested and assigned by user and be used and released according to user code


// basic allocator functions

// ----------------------------------------------------------------------------------------------------------
//  function     | effect                      | new ptr                     | old ptr            
// ----------------------------------------------------------------------------------------------------------
//  allocate     | alloc new memory            | new block                   | N/A
//  reallocate   | move memory to fit new size | same if fit, diff otherwise | intact but stale
// ----------------------------------------------------------------------------------------------------------


struct AllocatorGlobals;

using AllocId = uint32_t;

enum class Allocator: uint8_t
{
	none,
	system,
	app,
	stage,
	temp, // NOTE: everything after this must be temp allocators
	max,
};

static const constexpr size_t num_allocators = static_cast<size_t>(Allocator::max);

struct AllocHeader
{
	AllocId alloc_id{};
	uint32_t page{};
	size_t ptr{};
	size_t size{};
};

// TODO: maybe we should make this a template class to avoid void*
struct AllocHandle
{
	void* ptr{};
	AllocId alloc_id{};
	uint32_t header{};

	inline Allocator allocator_type() const;
	inline void* get() const { return ptr; }
	inline size_t capacity(const AllocatorGlobals& globals) const;
	inline bool valid(const AllocatorGlobals& globals) const;
};

struct AllocatorData
{
	std::vector<Buffer> pages;
	std::vector<AllocHeader> headers;
	size_t num_headers{};
	size_t free_header{};
	size_t curr_page{};
	AllocId curr_alloc_id = 1;
};

struct AllocatorGlobals
{
	AllocatorData allocators[num_allocators]{};
	Allocator current_temp_allocator = Allocator::temp;
	Allocator current_context_allocator = Allocator::none; // only one contextual allocator at a time (this enforces explicit allocation control, so you can be sure when you push a contextual allocator, all calls after than will use the same allocator)
	size_t context_allocator_push_count{}; // how many time the same contextual allocator gets pushed

	AllocHandle allocate(Allocator allocator, size_t size);
	void reallocate(AllocHandle& handle, size_t size);
	void deallocate(AllocHandle& handle);
	void regular_cleanup();
};

template<typename T>
concept Allocatable = requires (T o, Allocator allocator)
{ 
	o.alloc(allocator); 
};

inline const AllocHeader& access_alloc_header(const AllocatorGlobals& globals, uint32_t header_id)
{
	const size_t allocator = ((header_id >> 28) & 0x0000000F);
	const uint32_t header_idx = (header_id & 0x0FFFFFFF);
	asserts(allocator < num_allocators);
	const auto& allocator_data = globals.allocators[allocator];
	asserts(header_idx < allocator_data.num_headers);
	return allocator_data.headers[header_idx];
}

inline Allocator AllocHandle::allocator_type() const
{
	const size_t allocator_idx = ((header >> 28) & 0x0000000F);
	return static_cast<Allocator>(allocator_idx);
}

inline bool AllocHandle::valid(const AllocatorGlobals& globals) const
{
	if (alloc_id != 0)
	{
		const size_t allocator = ((header >> 28) & 0x0000000F);
		asserts(allocator < num_allocators);
		if (alloc_id == globals.allocators[allocator].curr_alloc_id)
		{
			return true;
		}
	}
	return false;
}

#if 0
inline void* AllocHandle::get(AllocatorGlobals& globals) const
{
	validate(globals);
	return ptr;
}
#endif

inline size_t AllocHandle::capacity(const AllocatorGlobals& globals) const
{
	asserts(valid(globals));
	return access_alloc_header(globals, header).size;
}


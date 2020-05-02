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

// ------------------------------------------------------------------------------------------------------------------------------------
//  function     | effect                      | new ptr                     | handle                  | ptr            
// ------------------------------------------------------------------------------------------------------------------------------------
//  allocate     | alloc new memory            | new block                   | N/A                     | N/A
//  deallocate   | mark memory reclaimable     | null                        | valid until reclaimed   | valid until reclaimed
//  reallocate   | move memory to fit new size | same if fit, diff otherwise | valid                   | not valid if moved, retrievable
//  reuse-alloc  | alloc memory over the old   | same if fit, diff otherwise | not valid               | not valid, not retrievable 
//  transallocate| move to fit and dealloc old | same if fit, diff otherwise | same if fit, or dealloc | valid if fit, or valid until reclaimed
// ------------------------------------------------------------------------------------------------------------------------------------


struct AllocatorGlobals;

using AllocId = uint32_t;

enum class Allocator
{
	persistent,
	temp1,
	temp2,
	string,
	max,
};

static const constexpr size_t num_allocators = static_cast<size_t>(Allocator::max);

struct AllocHeader
{
	AllocId alloc_id{};
	AllocId max_reg_id{};
	size_t ptr{};
	size_t size{};
};

struct AllocHandle
{
	void* ptr{};
	AllocId alloc_id{};
	uint32_t header{};

	inline Allocator allocator_type() const;
	inline void* get(AllocatorGlobals& globals) const;
	inline size_t capacity(AllocatorGlobals& globals) const;
	inline void validate(AllocatorGlobals& globals) const;
	void refresh(AllocatorGlobals& globals);
};

struct AllocatorData
{
	Buffer buffer;
	std::vector<AllocHeader> headers;
	size_t num_headers{};
	size_t free_header{};
	AllocId max_reg_id{};
};

struct AllocatorGlobals
{
	AllocId min_valid_ids[num_allocators]{};
	AllocatorData allocators[num_allocators]{};
	Allocator current_temp_allocator = Allocator::temp1;

	AllocHandle allocate(Allocator allocator, size_t size);
	void reallocate(AllocHandle& handle, size_t size);
	void transallocate(AllocHandle& handle, size_t size);
	void deallocate(AllocHandle& handle);
	void regular_cleanup();
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

inline void AllocHandle::validate(AllocatorGlobals& globals) const
{
	if (alloc_id != 0)
	{
		const size_t allocator = ((header >> 28) & 0x0000000F);
		asserts(allocator < num_allocators);
		if (alloc_id < globals.min_valid_ids[allocator])
		{
			const_cast<AllocHandle*>(this)->refresh(globals);
		}
	}
}

inline void* AllocHandle::get(AllocatorGlobals& globals) const
{
	validate(globals);
	return ptr;
}

inline size_t AllocHandle::capacity(AllocatorGlobals& globals) const
{
	validate(globals);
	return (alloc_id != 0 ? access_alloc_header(globals, header).size : 0);
}


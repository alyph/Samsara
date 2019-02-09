#pragma once

#include "assertion.h"
#include <cstdint>
#include <vector>

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
	AllocId realloc_id{};
	size_t ptr{};
	size_t size{};
};

struct AllocHandle
{
	void* ptr{};
	AllocId alloc_id{};
	uint32_t header{};

	inline void* get(AllocatorGlobals& globals) const;
	void refresh(AllocatorGlobals& globals);
};

struct AllocatorData
{
	AllocatorData() = default;
	~AllocatorData();

	uint8_t* data{};
	std::vector<AllocHeader> headers;
	size_t used_bytes{};
	size_t capacity{};
	size_t num_headers{};
	size_t free_header{};

};

struct AllocatorGlobals
{
	AllocId realloc_ids[num_allocators]{};
	AllocatorData allocators[num_allocators]{};
	Allocator current_temp_allocator = Allocator::temp1;

	AllocHandle allocate(Allocator allocator, size_t size);
};

inline void* AllocHandle::get(AllocatorGlobals& globals) const
{
	if (alloc_id != 0)
	{
		const size_t allocator = ((header >> 28) & 0x0000000F);
		asserts(allocator < num_allocators);
		if (alloc_id <= globals.realloc_ids[allocator])
		{
			const_cast<AllocHandle*>(this)->refresh(globals);
		}
	}
	return ptr;	
}
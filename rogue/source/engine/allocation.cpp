#include "allocation.h"
#include <cstdlib>
#include <algorithm>

static const constexpr size_t alloc_page_size = (1024 * 1024);
static const constexpr size_t header_page_size = 1024;

AllocatorData::~AllocatorData()
{
	std::free(data);
}

static void expand_allocator_to_fit(AllocatorData& allocator_data, AllocId& min_valid_id, size_t size)
{
	if (allocator_data.used_bytes + size > allocator_data.capacity)
	{
		const size_t num_pages_needed = 
			(allocator_data.used_bytes + size - allocator_data.capacity - 1) / alloc_page_size + 1;
		const size_t expand_size = alloc_page_size * num_pages_needed;
		if (allocator_data.data == nullptr)
		{
			allocator_data.data = reinterpret_cast<uint8_t*>(std::malloc(expand_size));
		}
		else
		{
			// TODO: should probably cache a max alloc/realloc id whenever alloc/realloc/refresh occurs
			// AllocId max_reg_id = 0;
			// for (size_t i = 0; i < allocator_data.num_headers; i++)
			// {
			// 	max_reg_id = std::max(max_reg_id, allocator_data.headers[i].max_reg_id);
			// }
			min_valid_id = std::max(allocator_data.max_reg_id + 1, min_valid_id); // TODO: probably doesn't need to + 1, anything <= realloc_id will be refreshed
			auto new_data = std::realloc(allocator_data.data, allocator_data.capacity + expand_size);
			asserts(new_data);
			allocator_data.data = reinterpret_cast<uint8_t*>(new_data);
		}
		allocator_data.capacity += expand_size;
	}
}

AllocHandle AllocatorGlobals::allocate(Allocator allocator, size_t size)
{
	const auto allocator_idx = static_cast<size_t>(allocator);
	asserts(allocator_idx < num_allocators);
	auto& allocator_data = allocators[allocator_idx];

	expand_allocator_to_fit(allocator_data, min_valid_ids[allocator_idx], size);
	size_t ptr = allocator_data.used_bytes;	
	allocator_data.used_bytes += size;

	// TODO: should we memzero the allocated memory?
	

	size_t header_idx{};
	if (allocator_data.num_headers < allocator_data.headers.size())
	{
		header_idx = allocator_data.num_headers++;
	}
	else if (allocator_data.free_header != 0)
	{
		asserts(false); // not impl
	}
	else
	{
		allocator_data.headers.resize(allocator_data.headers.size() + header_page_size);
		header_idx = allocator_data.num_headers++;
	}

	asserts(header_idx <= 0x0FFFFFFF);
	auto& header = allocator_data.headers[header_idx];
	// new allocation id must be higher than both the current header's previous ralloc id and the global realloc id
	// higher than previous header alloc id will invalidate all previous allocations
	// higher than global realloc id will make sure at least this allocation won't require an immediate refresh
	const AllocId new_alloc_id = std::max(min_valid_ids[allocator_idx], header.max_reg_id + 1);
	allocator_data.max_reg_id = std::max(allocator_data.max_reg_id, new_alloc_id);
	header.alloc_id = new_alloc_id;
	header.max_reg_id = new_alloc_id;
	header.ptr = ptr;
	header.size = size;

	AllocHandle handle;
	handle.ptr = (allocator_data.data + ptr);
	handle.alloc_id = new_alloc_id;
	handle.header = ((static_cast<uint32_t>(allocator_idx) << 28) | static_cast<uint32_t>(header_idx));
	return handle;
}

void AllocatorGlobals::reallocate(AllocHandle& handle, size_t size)
{
	handle.validate(*this);
	asserts(handle.alloc_id != 0);

	const size_t allocator = ((handle.header >> 28) & 0x0000000F);
	const uint32_t header_idx = (handle.header & 0x0FFFFFFF);
	asserts(allocator < num_allocators);
	auto& allocator_data = allocators[allocator];
	asserts(header_idx < allocator_data.num_headers);
	auto& header = allocator_data.headers[header_idx];
	asserts(header.size < size); // no point resizing if the block is already large enough

	if (header.ptr + header.size == allocator_data.used_bytes)
	{
		// this memory block is at the end of all allocations, so just need grow it
		size_t grow_size = (size - header.size);
		expand_allocator_to_fit(allocator_data, min_valid_ids[allocator], grow_size);
		allocator_data.used_bytes += grow_size;
		header.size = size;
		// handle and everything else remains the same
	}
	else
	{
		// this memory block is in the middle, move everything to a new block, mark old block free
		expand_allocator_to_fit(allocator_data, min_valid_ids[allocator], size);
		size_t ptr = allocator_data.used_bytes;	
		allocator_data.used_bytes += size;

		// copy old data to new location
		std::memcpy(allocator_data.data + ptr, allocator_data.data + header.ptr, header.size);

		// update header and handle
		const AllocId new_reg_id = std::max(min_valid_ids[allocator], header.max_reg_id + 1);
		min_valid_ids[allocator] = new_reg_id;
		allocator_data.max_reg_id = std::max(allocator_data.max_reg_id, new_reg_id);

		header.max_reg_id = new_reg_id;
		header.ptr = ptr;
		header.size = size;

		handle.ptr = (allocator_data.data + ptr);
		handle.alloc_id = new_reg_id;

		// TODO: mark old block free

	}	
}

void AllocatorGlobals::deallocate(AllocHandle& handle)
{
	handle.validate(*this);
	asserts(handle.alloc_id != 0);
	// deallocation doesn't really do anything, it only marks the allocation free and can be reclaimed during shrinking
	// TODO: mark the allocation free
	// TODO: maybe also mark header.alloc_id = 0 (maybe not see comments in refresh())
	// NOTE: the header may be reclaimable even before the memory is reclaimed 
	// because if header is used by other allocation, 
	// the refersh will detect it and clear the allocation handle
	// NOTE: min_valid_id would not need to be bumped, because the ptr is still valid
	// min_valid_id only need update if the ptr is no longer valid for access
}


void AllocHandle::refresh(AllocatorGlobals& globals)
{
	const size_t allocator = ((header >> 28) & 0x0000000F);
	const uint32_t header_idx = (header & 0x0FFFFFFF);
	asserts(allocator < num_allocators);
	auto& allocator_data = globals.allocators[allocator];
	asserts(header_idx < allocator_data.num_headers);
	auto& alloc_header = allocator_data.headers[header_idx];
	// TODO: maybe alloc_id == 0 is fine, which means the allocation is freed 
	// but the header is not yet used by other allocations
	// but that maybe bad because the earlier allocations may suddenly consider this header is valid as well
	if (alloc_id >= alloc_header.alloc_id && alloc_header.alloc_id != 0)
	{
		const auto min_valid_id = globals.min_valid_ids[allocator];
		if (alloc_header.max_reg_id < min_valid_id)
		{
			alloc_header.max_reg_id = min_valid_id;
			// min_valid_id should be <= max_reg_id already
			asserts(allocator_data.max_reg_id >= alloc_header.max_reg_id);
		}
		ptr = (allocator_data.data + alloc_header.ptr);
		alloc_id = alloc_header.max_reg_id;
	}
	else
	{
		// deallocated
		ptr = nullptr;
		alloc_id = 0;
		header = 0;
	}
}


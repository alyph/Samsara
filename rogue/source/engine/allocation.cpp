#include "allocation.h"
#include <cstdlib>
#include <algorithm>

static const constexpr size_t alloc_page_size = (1024 * 1024);
static const constexpr size_t header_page_size = 1024;

static size_t allocate_bytes(AllocatorData& allocator_data, AllocId& min_valid_id, size_t size)
{
	auto& buffer = allocator_data.buffer;
	auto old_data = buffer.get(0);
	const auto old_size = buffer.size();
	asserts(buffer.is_aligned(old_size));
	const auto new_size = buffer.get_next_aligned(old_size + size);
	buffer.resize(new_size, alloc_page_size);
	auto new_data = buffer.get(0);

	if (new_data != old_data)
	{
		min_valid_id = std::max(allocator_data.max_reg_id + 1, min_valid_id);
	}

	return (new_size - old_size);
}

AllocHandle AllocatorGlobals::allocate(Allocator allocator, size_t size)
{
	const auto allocator_idx = static_cast<size_t>(allocator);
	asserts(allocator_idx < num_allocators);
	auto& allocator_data = allocators[allocator_idx];

	size_t ptr = allocator_data.buffer.size();
	size_t alloc_size = allocate_bytes(allocator_data, min_valid_ids[allocator_idx], size);

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
	header.size = alloc_size;

	AllocHandle handle;
	handle.ptr = allocator_data.buffer.get(ptr);
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

	if (header.ptr + header.size == allocator_data.buffer.size())
	{
		// this memory block is at the end of all allocations, so just need grow it
		size_t grow_size = (size - header.size);
		header.size += allocate_bytes(allocator_data, min_valid_ids[allocator], grow_size);
		// handle and everything else remains the same
	}
	else
	{
		// this memory block is in the middle, move everything to a new block, mark old block free
		size_t ptr = allocator_data.buffer.size();
		size_t alloc_size = allocate_bytes(allocator_data, min_valid_ids[allocator], size);
		
		// copy old data to new location
		std::memcpy(allocator_data.buffer.get(ptr), allocator_data.buffer.get(header.ptr), header.size);

		// update header and handle
		const AllocId new_reg_id = std::max(min_valid_ids[allocator], header.max_reg_id + 1);
		min_valid_ids[allocator] = new_reg_id;
		allocator_data.max_reg_id = std::max(allocator_data.max_reg_id, new_reg_id);

		header.max_reg_id = new_reg_id;
		header.ptr = ptr;
		header.size = alloc_size;

		handle.ptr = allocator_data.buffer.get(ptr);
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

static void flush_temp_allocator(AllocatorGlobals& globals, Allocator allocator)
{
	const auto allocator_idx = static_cast<size_t>(allocator);
	auto& allocator_data = globals.allocators[allocator_idx];
	if (!allocator_data.buffer.empty() || allocator_data.num_headers > 0)
	{
		auto& min_valid_id = globals.min_valid_ids[allocator_idx];
		min_valid_id = std::max(allocator_data.max_reg_id + 1, min_valid_id);
		allocator_data.buffer.clear();
		allocator_data.num_headers = 0;
	}
}

void AllocatorGlobals::regular_cleanup()
{
	// TODO: temp1 and temp2 should be cleaned in an alternate fashion (welp rethink multi threaded situation when needed)
	flush_temp_allocator(*this, Allocator::temp1);
	flush_temp_allocator(*this, Allocator::temp2);
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
	// TODO: maybe we don't need mark alloc_id = 0 ever??
	if (alloc_id >= alloc_header.alloc_id && alloc_header.alloc_id != 0)
	{
		const auto min_valid_id = globals.min_valid_ids[allocator];
		if (alloc_header.max_reg_id < min_valid_id)
		{
			alloc_header.max_reg_id = min_valid_id;
			// min_valid_id should be <= max_reg_id already
			asserts(allocator_data.max_reg_id >= alloc_header.max_reg_id);
		}
		ptr = allocator_data.buffer.get(alloc_header.ptr);
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


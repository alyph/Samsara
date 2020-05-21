#include "allocation.h"
#include <cstdlib>
#include <algorithm>

static const constexpr size_t alloc_page_size = (1024 * 1024 * 4);
static const constexpr size_t header_page_size = 1024;

static size_t allocate_bytes(AllocatorData& alloc, size_t size)
{
	asserts(size > 0);

	size_t new_page = 0;
	if (alloc.curr_page < alloc.pages.size())
	{
		auto& buffer = alloc.pages[alloc.curr_page];
		const auto old_size = buffer.size();
		asserts(buffer.is_aligned(old_size));
		const auto new_size = buffer.get_next_aligned(old_size + size);
		if (new_size <= buffer.capacity())
		{
			buffer.resize(new_size);
			return old_size;
		}
		new_page = (alloc.curr_page + 1);
	}

	// look for new page
	const auto aligned_size = Buffer::get_next_aligned(size);
	size_t page_idx = new_page;
	for (; page_idx < alloc.pages.size(); page_idx++)
	{
		if (aligned_size <= alloc.pages[page_idx].capacity())
			break;
	}

	if (page_idx >= alloc.pages.size())
	{
		// create a new page
		alloc.pages.emplace_back().reserve(alloc_page_size * ((aligned_size + alloc_page_size - 1) / alloc_page_size));
		page_idx = (alloc.pages.size() - 1);
	}
	else
	{
		// found existing page that can fit
		// clear now since we might have not cleared it when discarded last time
		alloc.pages[page_idx].clear();
	}
	
	if (page_idx > new_page)
	{
		// swap this page back
		std::swap(alloc.pages[new_page], alloc.pages[page_idx]);
	}

	alloc.curr_page = new_page;
	auto& new_buffer = alloc.pages[new_page];
	asserts(aligned_size <= new_buffer.capacity());
	new_buffer.resize(aligned_size);
	return 0; // pointing to the beginning of the buffer
}

AllocHandle AllocatorGlobals::allocate(Allocator allocator, size_t size)
{
	// TODO: handle Allocator::none, maybe we should get idx = (allocator - 1)
	const auto allocator_idx = static_cast<size_t>(allocator);
	asserts(allocator_idx < num_allocators);
	auto& allocator_data = allocators[allocator_idx];

	const size_t ptr = allocate_bytes(allocator_data, size);
	const Buffer& buffer = allocator_data.pages[allocator_data.curr_page];
	const size_t alloc_size = (buffer.size() - ptr);

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
	const AllocId new_alloc_id = allocator_data.curr_alloc_id;
	header.alloc_id = new_alloc_id;
	header.ptr = ptr;
	header.size = alloc_size;
	header.page = static_cast<decltype(header.page)>(allocator_data.curr_page);

	AllocHandle handle;
	handle.ptr = buffer.get(ptr);
	handle.alloc_id = new_alloc_id;
	handle.header = ((static_cast<uint32_t>(allocator_idx) << 28) | static_cast<uint32_t>(header_idx));
	return handle;
}

void AllocatorGlobals::reallocate(AllocHandle& handle, size_t size)
{
	asserts(handle.valid(*this));

	const size_t allocator = ((handle.header >> 28) & 0x0000000F);
	const uint32_t header_idx = (handle.header & 0x0FFFFFFF);
	asserts(allocator < num_allocators);
	auto& allocator_data = allocators[allocator];
	asserts(header_idx < allocator_data.num_headers);
	auto& header = allocator_data.headers[header_idx];
	asserts(header.size < size); // no point resizing if the block is already large enough
	Buffer& buffer = allocator_data.pages[header.page];

	// TODO: code here is getting convoluted, see how it should reconcile with code in allocate_bytes
	const auto new_buffer_size = buffer.get_next_aligned(header.ptr + size);
	if ((header.page == allocator_data.curr_page) &&
		(header.ptr + header.size == buffer.size()) &&
		(new_buffer_size <= buffer.capacity()))
	{
		// this memory block is at the end of all allocations, so just need grow it
		size_t grow_size = (size - header.size);
		allocate_bytes(allocator_data, grow_size);
		asserts(buffer.size() == new_buffer_size);
		asserts(allocator_data.curr_page == header.page);
		header.size = (new_buffer_size - header.ptr);
		// handle and everything else remains the same
	}
	else
	{
		// this memory block is in the middle, move everything to a new block, mark old block free
		const size_t ptr = allocate_bytes(allocator_data, size);
		const Buffer& new_buffer = allocator_data.pages[allocator_data.curr_page];
		const size_t alloc_size = (new_buffer.size() - ptr);
		
		// copy old data to new location
		std::memcpy(new_buffer.get(ptr), buffer.get(header.ptr), header.size);

		// update header and handle
		header.ptr = ptr;
		header.size = alloc_size;

		handle.ptr = new_buffer.get(ptr);

		// TODO: mark old block free

	}	
}

#if 0
void AllocatorGlobals::transallocate(AllocHandle& handle, size_t size)
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
		// this memory block is in the middle, allocate new, copy content and deallocate old
		auto new_handle = allocate((Allocator)allocator, size);
		std::memcpy(new_handle.ptr, allocator_data.buffer.get(header.ptr), header.size);
		deallocate(handle);
		handle = new_handle;
	}
}
#endif

void AllocatorGlobals::deallocate(AllocHandle& handle)
{
	asserts(handle.valid(*this));
	// asserts(handle.alloc_id != 0);
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
	allocator_data.curr_page = 0;
	// only clear out the first page, the subsequent ones will be cleared when reached
	if (!allocator_data.pages.empty())
	{
		allocator_data.pages[0].clear();
	}
	// having used headers mean allocation occured
	if (allocator_data.num_headers > 0)
	{
		// reclaim all headers
		allocator_data.num_headers = 0;
		// bump the allocation id, so previous allocations all become invalid (which can be inspected for debugging purpose)
		allocator_data.curr_alloc_id++;
	}
}

void AllocatorGlobals::regular_cleanup()
{
	// TODO: temp1 and temp2 should be cleaned in an alternate fashion (welp rethink multi threaded situation when needed)
	// TODO: maybe have a flag for allocator that need regular cleanup or something
	flush_temp_allocator(*this, Allocator::temp);
}

#if 0
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
#endif


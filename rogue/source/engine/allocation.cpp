#include "allocation.h"
#include <cstdlib>
#include <algorithm>

static const constexpr size_t alloc_page_size = (1024 * 1024);
static const constexpr size_t header_page_size = 1024;

AllocatorData::~AllocatorData()
{
	std::free(data);
}

AllocHandle AllocatorGlobals::allocate(Allocator allocator, size_t size)
{
	const auto allocator_idx = static_cast<size_t>(allocator);
	asserts(allocator_idx < num_allocators);
	auto& allocator_data = allocators[allocator_idx];

	size_t ptr{};
	if (allocator_data.used_bytes + size <= allocator_data.capacity)
	{
		ptr = allocator_data.used_bytes;
	}
	else 
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
			AllocId max_realloc_id = realloc_ids[allocator_idx];
			for (size_t i = 0; i < allocator_data.num_headers; i++)
			{
				max_realloc_id = std::max(max_realloc_id, allocator_data.headers[i].realloc_id);
			}
			realloc_ids[allocator_idx] = (max_realloc_id + 1);

			auto new_data = std::realloc(allocator_data.data, allocator_data.capacity + expand_size);
			asserts(new_data);
			allocator_data.data = reinterpret_cast<uint8_t*>(new_data);
		}
		ptr = allocator_data.used_bytes;
		allocator_data.capacity += expand_size;
	}
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
	const AllocId new_alloc_id = std::max(realloc_ids[allocator_idx], header.realloc_id) + 1;
	header.alloc_id = new_alloc_id;
	header.realloc_id = new_alloc_id;
	header.ptr = ptr;
	header.size = size;

	AllocHandle handle;
	handle.ptr = (allocator_data.data + ptr);
	handle.alloc_id = new_alloc_id;
	handle.header = ((static_cast<uint32_t>(allocator_idx) << 28) | static_cast<uint32_t>(header_idx));
	return handle;
}

void AllocHandle::refresh(AllocatorGlobals& globals)
{
	const size_t allocator = ((header >> 28) & 0x0000000F);
	const uint32_t header_idx = (header & 0x0FFFFFFF);
	asserts(allocator < num_allocators);
	auto& allocator_data = globals.allocators[allocator];
	asserts(header_idx < allocator_data.headers.size());
	auto& alloc_header = allocator_data.headers[header_idx];
	if (alloc_id >= alloc_header.alloc_id && alloc_header.alloc_id != 0)
	{
		const auto realloc_id = globals.realloc_ids[allocator];
		if (alloc_header.realloc_id <= realloc_id)
		{
			alloc_header.realloc_id = (realloc_id + 1);
		}
		ptr = (allocator_data.data + alloc_header.ptr);
		alloc_id = alloc_header.realloc_id;
	}
	else
	{
		// deallocated
		ptr = nullptr;
		alloc_id = 0;
		header = 0;
	}
}
#include "string.h"

static const constexpr size_t sb_page_size = 1024;

void sb_expand_buffer(StringData& str_data, size_t size)
{
	// TODO: maybe attempt greedy method (greedy as in not allocate more than we need) first such that only 
	// expand for the given size if this is the last allocation of the allocator. we will not using paging 
	// until we are forced to move and copy memory once we are moved and at the end of the allocation then
	//  we can expand greedily again
	const auto curr_size = str_data.size();
	const size_t total_buffer_size = (sizeof(StringHeader) + curr_size + size);
	const size_t alloc_size = ((total_buffer_size + sb_page_size - 1) / sb_page_size) * sb_page_size;

	AllocatorGlobals& alloc_globals = engine().allocators;
	if (str_data.is_short())
	{
		// if the string is short, pull it into allocated memory first and then transallocate to expand
		// although this appears to have performed allocation twice, it is actually pretty cheap
		// since the transallocate after will simply be incrementing the size since the allocated
		// block here is at the end
		assign_normal_string(str_data, str_data.data(), curr_size, alloc_globals.current_temp_allocator);
	}
	// transallocation here is cheap if the buffer is at the end (which is the case most of the time)
	// don't use reallocation since reallocation will bump the alloc id as it needs to notify other
	// shared handles of the changed ptr, however in our case, there is no sharing of this handle
	alloc_globals.transallocate(str_data.normal_data.alloc_handle, alloc_size);
}



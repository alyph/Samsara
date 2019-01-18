#include "buffer.h"

static const constexpr size_t alloc_page_size = (1024 * 1024); // 1mb

BufferStore::BufferStore():
	data(alloc_page_size)
{
}

BufferHeader* BufferStore::allocate_data(uint32_t item_size, size_t size, size_t capacity)
{
	asserts(capacity >= size);

	BufferHeader* header{};
	if (free_header_index >= table.size())
	{
		header = &table.emplace_back();
		free_header_index = table.size();
	}
	else
	{
		header = &table[free_header_index];
		free_header_index = header->ptr; // ptr points to next free header
	}

	size_t alloc_bytes = (item_size * capacity);
	if (alloc_ptr + alloc_bytes > data.size())
	{
		data.resize(data.size() + alloc_page_size);
	}

	header->id = ++last_alloc_id;
	header->ptr = alloc_ptr;
	// header->data = data.data() + alloc_ptr;
	header->item_size = item_size;
	header->num_items = static_cast<decltype(header->num_items)>(size);
	header->capacity = static_cast<decltype(header->capacity)>(capacity);

	alloc_ptr += alloc_bytes;

	return header;
}
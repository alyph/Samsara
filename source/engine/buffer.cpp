#include "buffer.h"
#include <cstdlib>
#include <algorithm>

Buffer::~Buffer()
{
	free(data);
	_size = _capacity = 0;
}

static const constexpr size_t buffer_grow_factor = 2;
static const constexpr size_t buffer_grow_size = 1024;

void Buffer::resize(size_t new_size, size_t min_grow_size)
{
	if (new_size > _capacity)
	{
		size_t grow_size = std::max(buffer_grow_size, min_grow_size);
		size_t new_capacity = std::max(_capacity * buffer_grow_factor, _capacity + grow_size);
		if (new_size > new_capacity)
		{
			new_capacity += grow_size * (((new_size - new_capacity - 1) / grow_size) + 1);
		}

		reserve(new_capacity);
	}

	_size = new_size;	
}

void Buffer::reserve(size_t new_capacity)
{
	if (new_capacity > _capacity)
	{
		data = reinterpret_cast<uint8_t*>(std::realloc(data, new_capacity));
		asserts(data);
		asserts(is_aligned(reinterpret_cast<size_t>(data)));
		_capacity = new_capacity;
	}
}


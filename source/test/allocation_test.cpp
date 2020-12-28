#include "engine/engine.h"


static void verify_handle(const AllocHandle& handle, const AllocatorGlobals& allocators, Allocator allocator, size_t alloc_size)
{
	asserts(handle.get());
	asserts(handle.valid(allocators));
	asserts(handle.allocator_type() == allocator);
	asserts(handle.capacity(allocators) >= alloc_size);
}

int main()
{
	printf("Allocation Test Begin...\n");

	scoped_engine_init();

	const auto allocator = app_allocator();
	const size_t alloc_size = 32 * 1024 * 1024; // large enough to fill one page

	auto& allocators = engine().allocators;
	auto handle = allocators.allocate(allocator, alloc_size);
	verify_handle(handle, allocators, allocator, alloc_size);

	const size_t larger_size = 64 * 1024 * 1024;
	const auto old_handle = handle;
	allocators.reallocate(handle, larger_size);
	verify_handle(handle, allocators, allocator, larger_size);

	asserts(old_handle.get() != handle.get());
	asserts(old_handle.alloc_id == handle.alloc_id);
	asserts(old_handle.header == handle.header);

	auto temp_handle = allocators.allocate(temp_allocator(), alloc_size);
	verify_handle(temp_handle, allocators, temp_allocator(), alloc_size);

	allocators.regular_cleanup();
	asserts(!temp_handle.valid(allocators));

	printf("Allocation Test Done.\n");
}










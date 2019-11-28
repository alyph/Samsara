#include "singleton.h"
#include "assertion.h"
#include <algorithm>
#include <malloc.h>

struct SingletonRegistry
{
	size_t ptr{};
	size_t size{};
	std::unique_ptr<ISingletonFactory> factory;
};

static std::vector<SingletonRegistry> registered_singletons;
static size_t singleton_total_size = 0;
static size_t max_alignment = 0;
static bool closed_for_registration = false;

size_t register_singleton(size_t size, size_t alignment, std::unique_ptr<ISingletonFactory>&& factory)
{
	asserts(!closed_for_registration);
	asserts(size > 0 && alignment > 0);
	asserts(factory);
	auto ptr = ((singleton_total_size + alignment - 1) & (~(alignment - 1)));
	singleton_total_size = (ptr + size);
	max_alignment = std::max(max_alignment, alignment);
	auto& registry = registered_singletons.emplace_back();
	registry.ptr = ptr;
	registry.size = size;
	registry.factory = std::move(factory);	
	return ptr;
}

SingletonCollection::SingletonCollection()
{
	closed_for_registration = true;
	if (!registered_singletons.empty())
	{
		asserts(singleton_total_size > 0 && max_alignment > 0);
		const size_t aligned_size = ((singleton_total_size + max_alignment - 1) & (~(max_alignment - 1)));
		buffer = reinterpret_cast<uint8_t*>(_aligned_malloc(aligned_size, max_alignment));
		for (const auto& registry : registered_singletons)
		{
			registry.factory->construct(buffer + registry.ptr);
		}
	}
}

SingletonCollection::~SingletonCollection()
{
	for (const auto& registry : registered_singletons)
	{
		registry.factory->destruct(buffer + registry.ptr);
	}
	_aligned_free(buffer);
}
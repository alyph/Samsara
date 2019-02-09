#include "singleton.h"
#include "assertion.h"

struct SingletonRegistry
{
	size_t ptr{};
	size_t size{};
	std::unique_ptr<ISingletonFactory> factory;
};

static std::vector<SingletonRegistry> registered_singletons;
static size_t singleton_total_size = 0;
static bool closed_for_registration = false;

size_t register_singleton(size_t size, std::unique_ptr<ISingletonFactory>&& factory)
{
	asserts(!closed_for_registration);
	asserts(size > 0);
	asserts(factory);
	auto ptr = singleton_total_size;
	singleton_total_size += size;
	auto& registry = registered_singletons.emplace_back();
	registry.ptr = ptr;
	registry.size = size;
	registry.factory = std::move(factory);	
	return ptr;
}

SingletonCollection::SingletonCollection()
{
	closed_for_registration = true;
	buffer.resize(singleton_total_size);
	for (const auto& registry : registered_singletons)
	{
		registry.factory->construct(buffer.data() + registry.ptr);
	}
}

SingletonCollection::~SingletonCollection()
{
	for (const auto& registry : registered_singletons)
	{
		registry.factory->destruct(buffer.data() + registry.ptr);
	}
}
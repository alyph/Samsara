#pragma once

// #include "buffer.h"
#include "singleton.h"
#include "assertion.h"
#include "allocation.h"
#include "macros.h"
#include <type_traits>

class Window;

struct Engine
{
	// Engine() = default;
	// Engine(const Engine&) = delete;
	SingletonCollection singletons;
	AllocatorGlobals allocators;

	Window* window{};
};

struct EngineStore
{
	~EngineStore();
	bool initialized = false;
	std::aligned_storage_t<sizeof(Engine), alignof(Engine)> engine_buffer;
};

extern EngineStore global_engine;

static inline Engine& engine()
{
	asserts(global_engine.initialized);
	return *reinterpret_cast<Engine*>(&global_engine.engine_buffer);
}

struct ScopedEngineInitializer
{
	ScopedEngineInitializer();
	~ScopedEngineInitializer();
};

#define scoped_engine_init() ScopedEngineInitializer CONCAT(_engine_init_, __COUNTER__){}

inline Allocator system_allocator()
{
	return Allocator::system;
}

inline Allocator app_allocator()
{
	return Allocator::app;
}

inline Allocator stage_allocator()
{
	return Allocator::stage;
}

inline Allocator temp_allocator()
{
	return engine().allocators.current_temp_allocator;
}

inline Allocator perm_allocator()
{
	const auto& alloc_globals = engine().allocators;
	return (alloc_globals.perm_allocator_stack_size > 0 ?
		alloc_globals.perm_allocator_stack[alloc_globals.perm_allocator_stack_size-1] : Allocator::none);
}

inline void push_perm_allocator(Allocator alloc)
{
	auto& alloc_globals = engine().allocators;
	asserts(alloc_globals.perm_allocator_stack_size < max_perm_allocator_stack_size);
	alloc_globals.perm_allocator_stack[alloc_globals.perm_allocator_stack_size] = alloc;
	alloc_globals.perm_allocator_stack_size++;
}

inline void pop_perm_allocator()
{
	auto& alloc_globals = engine().allocators;
	asserts(alloc_globals.perm_allocator_stack_size > 0);
	alloc_globals.perm_allocator_stack_size--;
}


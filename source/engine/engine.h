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

inline Allocator context_allocator()
{
	return engine().allocators.current_context_allocator;
}

// you can keep pushing the same allocator on to the stack
// but you cannot change it if there's one already set
// this is so whoever sets the context allocator will 100% sure
// all subsequent calls will allocate to the set allocator (if using context allocator)
inline void push_context_allocator(Allocator alloc)
{
	asserts(alloc != Allocator::none);
	auto& alloc_globals = engine().allocators;
	asserts(alloc_globals.current_context_allocator == Allocator::none || alloc_globals.current_context_allocator == alloc);
	alloc_globals.current_context_allocator = alloc;
	alloc_globals.context_allocator_push_count++;
}

inline void pop_context_allocator()
{
	auto& alloc_globals = engine().allocators;
	asserts(alloc_globals.context_allocator_push_count > 0);
	alloc_globals.context_allocator_push_count--;
	if (alloc_globals.context_allocator_push_count == 0)
	{
		alloc_globals.current_context_allocator = Allocator::none;
	}
}

struct ScopedContextAllocator
{
	inline ScopedContextAllocator(Allocator alloc) { push_context_allocator(alloc); }
	inline ~ScopedContextAllocator() { pop_context_allocator(); }
};

#define scoped_context_allocator(alloc) ScopedContextAllocator CONCAT(_engine_context_allocator_, __COUNTER__){alloc}


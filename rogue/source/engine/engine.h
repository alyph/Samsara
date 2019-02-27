#pragma once

// #include "buffer.h"
#include "singleton.h"
#include "assertion.h"
#include "allocation.h"
#include "macros.h"
#include <type_traits>

struct Engine
{
	// Engine() = default;
	// Engine(const Engine&) = delete;
	SingletonCollection singletons;
	AllocatorGlobals allocators;
};

// TODO: use https://en.cppreference.com/w/cpp/types/aligned_storage

struct EngineStore
{
	~EngineStore();
	std::aligned_storage_t<sizeof(Engine), alignof(Engine)> engine_buffer;
	bool initialized = false;
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

#define scoped_engine_init() ScopedEngineInitializer CONCAT(engine_, __COUNTER__){}
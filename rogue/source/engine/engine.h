#pragma once

// #include "buffer.h"
#include "singleton.h"
#include "assertion.h"
#include "allocation.h"

struct Engine
{
	// Engine() = default;
	// Engine(const Engine&) = delete;
	SingletonCollection singletons;
	AllocatorGlobals allocators;
};

extern Engine* global_engine;

static inline Engine& engine()
{
	asserts(global_engine);
	return *global_engine;
}

static inline void set_engine(Engine& in_engine)
{
	asserts(!global_engine);
	global_engine = &in_engine;
}

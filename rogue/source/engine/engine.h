#pragma once

#include "buffer.h"
#include "singleton.h"
#include "assertion.h"

struct Engine
{
	// Engine() = default;
	// Engine(const Engine&) = delete;
	BufferStore buffer_store;
	SingletonCollection singletons;
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

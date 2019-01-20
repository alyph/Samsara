#pragma once

#include "buffer.h"
#include "singleton.h"

struct Engine
{
	// Engine() = default;
	// Engine(const Engine&) = delete;
	BufferStore buffer_store;
	SingletonCollection singletons;
};
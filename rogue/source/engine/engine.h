#pragma once

#include "buffer.h"
#include "singleton.h"

struct EngineData
{
	BufferStore buffer_store;
	SingletonCollection singletons;
};
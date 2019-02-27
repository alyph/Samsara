#include "engine.h"

EngineStore global_engine{};

EngineStore::~EngineStore()
{
	asserts(!initialized, "term_engine() must be called before shutdown");
}

ScopedEngineInitializer::ScopedEngineInitializer()
{
	asserts(!global_engine.initialized);
	new (&global_engine.engine_buffer) Engine();
	global_engine.initialized = true;
}

ScopedEngineInitializer::~ScopedEngineInitializer()
{
	asserts(global_engine.initialized);
	reinterpret_cast<Engine*>(&global_engine.engine_buffer)->~Engine();
	global_engine.initialized = false;
}

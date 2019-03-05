#include "engine.h"

EngineStore global_engine{};

EngineStore::~EngineStore()
{
	asserts(!initialized, "incorrect engine init calls occured. call scoped_engine_init() in main()");
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

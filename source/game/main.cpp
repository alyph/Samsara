#include "game.h"
#include "engine/app.h"

int main()
{
	AppConfig config;
	config.window_width = 1920;
	config.window_height = 1080;

	return run_app<Game>(config);
}


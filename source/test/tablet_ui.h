#pragma once

#include "engine/presenter.h"
#include "engine/texture.h"
#include "engine/time.h"

class TabletUIApp
{
public:
	TabletUIApp();
	void update(const Time& time);
	bool ended();
	void present(const Context& ctx);
	void shutdown() {}

private:
	Id tablet_shader;
	Id tablet_screen_shader;
	Id atlas_texture;
	int click_count{};
};